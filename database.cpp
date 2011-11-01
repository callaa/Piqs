#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDebug>
#include <QVariant>
#include <QMessageBox>

#include "database.h"
#include "picture.h"
#include "tags.h"

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
		// Make sure the necessary tables exist	delete m_tags;

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

		m_tags = new Tags(this);
		m_tags->createTables();
		m_tags->reload();

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

void Database::showError(const QString& message, const QSqlQuery &query)
{
	QMessageBox msg(QMessageBox::Critical, tr("Database error"), message, QMessageBox::Ok);
	msg.setDetailedText(query.lastError().text());
	msg.exec();
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

