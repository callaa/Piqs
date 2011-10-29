#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
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
	m_dbname = QString("db") + QString::number(dbindex);
	qDebug() << "Opening database connection" << m_dbname;

	m_db = QSqlDatabase::addDatabase("QSQLITE", m_dbname);
	m_db.setDatabaseName(metadir.absoluteFilePath("index.db"));

	if(m_db.open()) {
		// Make sure the necessary tables exist
		QStringList tables = m_db.tables();

		// The main picture list table
		if(!tables.contains("picture")) {
			qDebug() << "Picture table does not exist. Creating...";
			QSqlQuery q(m_db);
			q.exec("CREATE TABLE picture ("
				   "picid INTEGER PRIMARY KEY NOT NULL,"
				   "filename TEXT UNIQUE NOT NULL,"
				   "hidden INTEGER NOT NULL,"
				   "title TEXT NOT NULL,"
				   "tags TEXT NOT NULL,"
				   "rotation INTEGER NOT NULL"
				   ")");
		}

		// Tags
		if(!tables.contains("tag")) {
			qDebug() << "Tag table does not exist. Creating...";
			QSqlQuery q(m_db);
			q.exec("CREATE TABLE tag ("
				   "tagid INTEGER PRIMARY KEY NOT NULL,"
				   "tag TEXT UNIQUE NOT NULL"
				   ")");

		}

		// Tag <-> picture associations
		if(!tables.contains("tagmap")) {
			qDebug() << "Tag association map does not exist. Creating...";
			QSqlQuery q(m_db);
			q.exec("CREATE TABLE tagmap ("
				   "picid INTEGER NOT NULL,"
				   "tagid INTEGER NOT NULL,"
				   "tagset INTEGER NOT NULL,"
				   "PRIMARY KEY (picid, tagid, tagset),"
				   "FOREIGN KEY (picid) REFERENCES picture ON DELETE CASCADE ON UPDATE CASCADE,"
				   "FOREIGN KEY (tagid) REFERENCES tag ON DELETE CASCADE ON UPDATE CASCADE"
				   ")");
		}

		// General program options
		if(!tables.contains("option")) {
			QSqlQuery q(m_db);
			q.exec("CREATE TABLE option ("
				   "optkey TEXT PRIMARY KEY NOT NULL,"
				   "optvalue TEXT NOT NULL"
				   ")");
		}
	}
}

Database::~Database()
{
}

void Database::saveSetting(const QString& key, const QVariant& value) const
{
	QSqlQuery q(m_db);
	q.prepare("INSERT OR REPLACE INTO option (optkey, optvalue) VALUES (?, ?)");
	q.addBindValue(key);
	q.addBindValue(value);
	if(!q.exec())
		qDebug() << "Couldn't save configuration key" << key << ": " << q.lastError().text();
}

//! Get a configuration value
QVariant Database::getSetting(const QString& key) const
{
	QSqlQuery q(m_db);
	q.prepare("SELECT optvalue FROM option WHERE optkey=?");
	q.addBindValue(key);
	q.exec();
	if(q.next())
		return q.value(0);
	return QVariant();
}

/**
  \return tag ID or -1 in case of error
  */
int Database::getOrCreateTag(const QString& name) const
{
	QString normalized = name.toLower();
	if(normalized.length()==0)
		return -1;

	QSqlQuery q(m_db);
	q.prepare("SELECT tagid FROM tag WHERE tag=?");
	q.addBindValue(normalized);
	if(!q.exec()) {
		qDebug() << "Couldn't get ID for tag" << name << q.lastError().text();
		return -1;
	}
	if(q.next())
		return q.value(0).toInt();

	q.prepare("INSERT INTO tag (tag) VALUES (?)");
	q.addBindValue(normalized);
	if(!q.exec()) {
		qDebug() << "Couldn't insert tag" << name << q.lastError().text();
		return -1;
	}

	return q.lastInsertId().toInt();
}

/**
  \return tag ID or -1 if not found
  */
int Database::getTag(const QString& name) const
{
	QString normalized = name.toLower();

	QSqlQuery q(m_db);
	q.prepare("SELECT tagid FROM tag WHERE tag=?");
	q.addBindValue(normalized);
	if(!q.exec())
		qDebug() << "Couldn't get ID for tag" << name << q.lastError().text();
	if(q.next())
		return q.value(0).toInt();

	return -1;
}
