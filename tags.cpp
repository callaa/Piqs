#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QSqlError>
#include <QCompleter>

#include "tags.h"
#include "database.h"
#include "util.h"

Tags::Tags(Database *parent) :
	QAbstractListModel(parent), m_database(parent)
{
}

int Tags::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_tags.count();
}

QVariant Tags::data(const QModelIndex &index, int role) const
{
	if(index.row() >= 0 && index.row() < rowCount(index)) {
		if(role==Qt::EditRole || role==Qt::DisplayRole) {
			return m_tags.at(index.row());
		}
	}

	return QVariant();
}

/**
  Tag index creation is in its own function, because when rebuilding
  the index, its a good idea to just drop the old data, tables and all.
  \param dropfirst if true, the old tables are dropped
  */
void Tags::createTables(bool dropfirst)
{
	QSqlQuery q(m_database->get());

	if(dropfirst) {
		if(!q.exec("DROP TABLE IF EXISTS tagmap"))
			qDebug() << "Couldn't drop tagmap:" << q.lastError().text();
		if(!q.exec("DROP TABLE IF EXISTS tag"))
			qDebug() << "Couldn't drop tag table:" << q.lastError().text();

		beginResetModel();
		m_tags.clear();
		m_taghash.clear();
		endResetModel();
	}

	// Tags
	q.exec("CREATE TABLE IF NOT EXISTS tag ("
		   "tagid INTEGER PRIMARY KEY NOT NULL,"
		   "tag TEXT UNIQUE NOT NULL"
		   ")");

	// Tag <-> picture associations
	q.exec("CREATE TABLE IF NOT EXISTS tagmap ("
		   "picid INTEGER NOT NULL,"
		   "tagid INTEGER NOT NULL,"
		   "tagset INTEGER NOT NULL,"
		   "PRIMARY KEY (picid, tagid, tagset),"
		   "FOREIGN KEY (picid) REFERENCES picture ON DELETE CASCADE ON UPDATE CASCADE,"
		   "FOREIGN KEY (tagid) REFERENCES tag ON DELETE CASCADE ON UPDATE CASCADE"
		   ")");

}

void Tags::reload()
{
	beginResetModel();
	m_tags.clear();
	m_taghash.clear();

	QSqlQuery q("SELECT tagid, tag FROM tag ORDER BY tag ASC", m_database->get());
	while(q.next()) {
		m_tags.append(q.value(1).toString());
		m_taghash.insert(q.value(1).toString(), q.value(0).toInt());
	}

	endResetModel();
}

/**
  \return tag ID or -1 in case of error
  */
int Tags::getOrCreate(const QString& name)
{
	QString normalized = Util::cleanTagName(name);
	if(normalized.length()==0)
		return -1;

	QSqlQuery q(m_database->get());

	// See if the tag has been aliased
	q.prepare("SELECT tag FROM tagalias WHERE alias=?");
	q.bindValue(0, normalized);
	if(!q.exec())
		qDebug() << "Couldn't get ID for tag" << name << q.lastError().text();
	if(q.next())
		normalized = q.value(0).toString();

	// Okay, we got an alias. Now get the real tag
	int tag = m_taghash.value(normalized);
	if(tag>0)
		return tag;

	// If it does not exist, create it
	q.prepare("INSERT INTO tag (tag) VALUES (?)");
	q.addBindValue(normalized);
	if(!q.exec()) {
		qDebug() << "Couldn't insert tag" << name << q.lastError().text();
		return -1;
	}

	tag = q.lastInsertId().toInt();

	beginInsertRows(QModelIndex(), m_tags.count(), m_tags.count());
	m_tags.append(normalized);
	m_taghash.insert(normalized, tag);
	endInsertRows();

	return tag;
}

/**
  \return tag ID or -1 if not found
  */
int Tags::get(const QString& name) const
{
	QString normalized = Util::cleanTagName(name);

	QSqlQuery q(m_database->get());
	// See if the tag has been aliased
	q.prepare("SELECT tag FROM tagalias WHERE alias=?");
	q.bindValue(0, normalized);
	if(!q.exec())
		qDebug() << "Couldn't get ID for tag" << name << q.lastError().text();
	if(q.next())
		normalized = q.value(0).toString();

	return m_taghash.value(normalized, -1);
}
