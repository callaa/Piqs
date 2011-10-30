#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDebug>
#include <QVariant>

#include "database.h"
#include "picture.h"
#include "util.h"

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

		createTagIndexTables();

		// Tag aliases
		if(!tables.contains("tagalias")) {
			qDebug() << "Tag alias table does not exist. Creating...";
			QSqlQuery q(m_db);
			q.exec("CREATE TABLE tagalias ("
				   "alias TEXT PRIMARY KEY NOT NULL,"
				   "tag TEXT NOT NULL"
				   ")");
		}

		// Tag implication rules
		if(!tables.contains("tagrule")) {
			qDebug() << "Tag rule table does not exist. Creating...";
			QSqlQuery q(m_db);
			q.exec("CREATE TABLE tagrule ("
				   "rule TEXT PRIMARY KEY NOT NULL,"
				   "ruleorder INTEGER NOT NULL,"
				   "tags TEXT NOT NULL"
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

/**
  Tag index creation is in its own function, because when rebuilding
  the index, its a good idea to just drop the old data, tables and all.
  \param dropfirst if true, the old tables are dropped
  */
void Database::createTagIndexTables(bool dropfirst) const
{
	QSqlQuery q(m_db);
	// Tags
	if(dropfirst)
		q.exec("DROP TABLE IF EXISTS tag");
	q.exec("CREATE TABLE IF NOT EXISTS tag ("
		   "tagid INTEGER PRIMARY KEY NOT NULL,"
		   "tag TEXT UNIQUE NOT NULL"
		   ")");

	// Tag <-> picture associations
	if(dropfirst)
		q.exec("DROP TABLE IF EXISTS tagmap");
	q.exec("CREATE TABLE IF NOT EXISTS tagmap ("
		   "picid INTEGER NOT NULL,"
		   "tagid INTEGER NOT NULL,"
		   "tagset INTEGER NOT NULL,"
		   "PRIMARY KEY (picid, tagid, tagset),"
		   "FOREIGN KEY (picid) REFERENCES picture ON DELETE CASCADE ON UPDATE CASCADE,"
		   "FOREIGN KEY (tagid) REFERENCES tag ON DELETE CASCADE ON UPDATE CASCADE"
		   ")");
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
	QString normalized = Util::cleanTagName(name);
	if(normalized.length()==0)
		return -1;

	// See if tag exists
	int tagid = getTag(normalized);

	if(tagid>0)
		return tagid;

	// If not, create it
	QSqlQuery q(m_db);
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
	QString normalized = Util::cleanTagName(name);

	QSqlQuery q(m_db);
	// See if the tag has been aliased
	q.prepare("SELECT tag FROM tagalias WHERE alias=?");
	q.bindValue(0, normalized);
	if(!q.exec())
		qDebug() << "Couldn't get ID for tag" << name << q.lastError().text();
	if(q.next())
		normalized = q.value(0).toString();

	// Okay, we got an alias. Now get the real tag
	q.prepare("SELECT tagid FROM tag WHERE tag=?");
	q.bindValue(0, normalized);
	if(!q.exec())
		qDebug() << "Couldn't get ID for tag" << name << q.lastError().text();
	if(q.next()) {
		// Tag found
		return q.value(0).toInt();
	}

	return -1;
}
