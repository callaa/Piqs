#include <QDebug>
#include <QImageReader>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

#include "rescanthread.h"

#include "gallery.h"
#include "database.h"

RescanThread::RescanThread(const Gallery *gallery, QObject *parent) :
	QThread(parent), m_gallery(gallery), m_count(0), m_abortflag(false)
{

}

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
			qDebug() << "Couldn't open clone database!";
			return;
		}

		QSqlQuery q(db);
		q.prepare("INSERT OR IGNORE INTO picture (filename, hidden, title, tags) VALUES (?, 0, \"\", \"\")");

		// Depth first search of all subdirectories
		rescan(filefilter, QString(), m_gallery->root(), q);
	}

	QSqlDatabase::removeDatabase(dbname);
}

void RescanThread::rescan(const QStringList& filefilter, const QString& prefix, const QDir& root, QSqlQuery& query)
{
	// Recursively scan subdirectories
	QStringList dirs = root.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach(const QString& name, dirs) {
		if(m_abortflag)
			return;
		rescan(filefilter, prefix + name + QDir::separator(), QDir(root.absoluteFilePath(name)), query);
	}

	// Scan supported images
	QFileInfoList files = root.entryInfoList(filefilter, QDir::Files | QDir::Readable);
	foreach(const QFileInfo& file, files) {
		if(m_abortflag)
			return;
		QString path = prefix + file.fileName();
		query.bindValue(0, path);
		if(!query.exec())
			qDebug() << "Couldn't insert file:" << query.lastError().text();

		emit filesAdded(++m_count);
	}

}

void RescanThread::abortScan()
{
	m_abortflag = true;
}
