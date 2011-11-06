#include <QDebug>
#include <QImageReader>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QPair>

#include "rescanthread.h"

#include "gallery.h"
#include "database.h"
#include "util.h"

static const qint64 REFRESH_PERIOD = 1000;

RescanThread::RescanThread(const Gallery *gallery, QObject *parent) :
	QThread(parent), m_gallery(gallery), m_filecount(0), m_foldercount(0), m_abortflag(false), m_time(0)
{

}

typedef QPair<int, QString> MovedPicture;

void RescanThread::run() {
	// Get a list of image formats supported by Qt
	QStringList filefilter;
	foreach(QByteArray format, QImageReader::supportedImageFormats()) {
		QString fmt = (QString("*.") + format).toLower();
		if(!filefilter.contains(fmt))
			filefilter << fmt;
	}

	QString dbname = QString("rescan") + m_gallery->database()->name();

	{
		// Create a clone of the gallery database for use in this thread
		QSqlDatabase db = QSqlDatabase::cloneDatabase(m_gallery->database()->get(), dbname);
		if(!db.open()) {
			emit statusChanged(tr("Couldn't open database!"));
			qDebug() << "Couldn't open clone database!";
			return;
		}

		QSqlQuery q(db);

		// Remember the latest ID before scanning for new pictures
		int latest = 0;
		q.exec("SELECT MAX(picid) FROM picture");
		if(q.next())
			latest = q.value(0).toInt();

		q.exec(QString("INSERT OR REPLACE INTO option (optkey, optvalue) VALUES ('lastnewid', %1)").arg(latest));

		// Mark all existing pictures as "not found". The pictures that haven't been removed from
		// disk will be marked back as found as we scan the file system.
		q.exec("UPDATE picture SET found=0");

		q.prepare("INSERT INTO picture (filename, hidden, title, tags, rotation, found, hash) VALUES (?, 0, \"\", \"\", 0, 1, ?)");

		{
			QSqlQuery q2(db);
			q2.prepare("UPDATE picture SET found=1, hash=? WHERE filename=?");

			// Depth first search of all subdirectories
			rescan(filefilter, QString(), m_gallery->root(), q, q2);
		}

		// Update status display
		emit foldersSearched(m_foldercount);
		emit filesAdded(m_filecount);

		// Find duplicate images
		emit statusChanged(tr("Searching for duplicate images..."));
		q.exec("DELETE FROM duplicate");
		q.exec("INSERT INTO duplicate SELECT picid FROM picture WHERE hash IN (SELECT hash FROM picture WHERE hash!=\"\" GROUP BY hash HAVING(COUNT(hash)>1))");

		// See if any of the missing files were moved
		int moves = 0;
		q.exec("SELECT picid, hash FROM picture JOIN duplicate USING (picid) WHERE found=0");
		QList<MovedPicture> moved;
		while(q.next())
			moved.append(MovedPicture(q.value(0).toInt(), q.value(1).toString()));

		// Rename the pictures if the target picture appears new
		foreach(const MovedPicture& mp, moved) {
			q.prepare("SELECT picid FROM picture LEFT JOIN tagmap USING (picid) WHERE picid!=? AND hash=? GROUP BY picid HAVING COUNT(tagid)=0");
			q.bindValue(0, mp.first);
			q.bindValue(1, mp.second);
			q.exec();
			qDebug() << "move?" << mp.first << mp.second;
			if(q.next()) {
				qDebug() << "move" << q.value(0) << q.value(1);
				int newid = q.value(0).toInt();
				// Get old metadata
				q.exec("SELECT hidden, title, rotation FROM picture WHERE picid=" + QString::number(mp.first));
				q.next();
				QVariant hidden = q.value(0);
				QVariant title = q.value(1);
				QVariant rotation = q.value(2);

				// Update new picture and delete old one
				q.prepare("UPDATE picture SET hidden=?, title=?, rotation=? WHERE picid=?");
				q.bindValue(0, hidden);
				q.bindValue(1, title);
				q.bindValue(2, rotation);
				q.bindValue(3, newid);
				q.exec();

				q.exec("UPDATE tagmap SET picid=" + QString::number(newid) + " WHERE picid=" + QString::number(mp.first));
				q.exec("DELETE FROM picture WHERE picid=" + QString::number(mp.first));
				q.exec("DELETE FROM duplicate WHERE picid=" + QString::number(newid));
				++moves;
			}
		}

		// Count missing and duplicate files
		int missing = 0;
		q.exec("SELECT COUNT(picid) FROM picture WHERE found=0");
		if(q.next())
			missing = q.value(0).toInt();

		int dupes=0;
		q.exec("SELECT COUNT(picid) FROM duplicate");
		if(q.next())
			dupes = q.value(0).toInt();

		emit dupesFound(dupes);
		emit missingFound(missing);
		emit movesFound(moves);
	}

	emit statusChanged(tr("Done."));
	QSqlDatabase::removeDatabase(dbname);
}

/**
  Check if the status signal refresh period has elapsed.
  */
bool RescanThread::periodElapsed()
{
#if QT_VERSION >= 0x040700
	qint64 time = QDateTime::currentMSecsSinceEpoch();
	if(time - 1000 > m_time) {
		m_time = time;
		return true;
	}
	return false;
#else
	return true;
#endif
}

void RescanThread::rescan(const QStringList& filefilter, const QString& prefix, const QDir& root, QSqlQuery& insertquery, QSqlQuery& updatequery)
{
	// Recursively scan subdirectories
	QStringList dirs = root.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach(const QString& name, dirs) {
		if(m_abortflag)
			return;
		++m_foldercount;
		if(periodElapsed())
			emit foldersSearched(m_foldercount);
		rescan(filefilter, prefix + name + QDir::separator(), QDir(root.absoluteFilePath(name)), insertquery, updatequery);
	}

	// Scan supported images
	QFileInfoList files = root.entryInfoList(filefilter, QDir::Files | QDir::Readable);
	foreach(const QFileInfo& file, files) {
		if(m_abortflag)
			return;

		QString path = prefix + file.fileName();
		QString hash = Util::hashFile(file.absoluteFilePath());
		insertquery.bindValue(0, path);
		insertquery.bindValue(1, hash);
		if(!insertquery.exec()) {
			// Insert failed, most likely because the file was already in the database.

			updatequery.bindValue(0, hash);
			updatequery.bindValue(1, path);
			if(!updatequery.exec())
				qDebug() << "Couldn't insert or update file:" << updatequery.lastError().text();
		}

		// Limit the file count update rate
		++m_filecount;
		if(periodElapsed())
			emit filesAdded(m_filecount);
	}
}

void RescanThread::abortScan()
{
	m_abortflag = true;
}
