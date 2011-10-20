#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDir>
#include <QDebug>
#include <QVariant>

#include "database.h"
#include "picture.h"

int Database::dbindex = 0;

Database::Database(const QDir& metadir, QObject *parent) :
    QObject(parent)
{
	++dbindex;
	_dbname = QString("db") + QString::number(dbindex);
	qDebug() << "Opening database connection" << _dbname;

	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", _dbname);
	db.setDatabaseName(metadir.absoluteFilePath("index.db"));
	_open = db.open();

	if(_open) {
		// Make sure the necessary tables exist
		QStringList tables = db.tables();
		if(!tables.contains("picture")) {
			qDebug() << "Picture table does not exist. Creating...";
			QSqlQuery q(db);
			q.exec("CREATE TABLE picture ("
				   "id INTEGER PRIMARY KEY,"
				   "filename UNIQUE NOT NULL"
				   ")"
				   );
		}
	}
}

void Database::syncPictures(const QList<Picture> &pictures)
{
	if(!_open)
		return;

	// Insert missing pictures
	QSqlQuery q(QSqlDatabase::database(_dbname));
	q.prepare("INSERT OR IGNORE INTO picture (filename) VALUES (?)");
	foreach(Picture p, pictures) {
		q.bindValue(0, p.relativeName());
		qDebug() << "inserting" << p.relativeName();
		q.exec();
	}
}
