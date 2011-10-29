#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

#include "thumbnailmodel.h"
#include "iconcache.h"
#include "gallery.h"
#include "tagquery.h"

ThumbnailModel::ThumbnailModel(const Gallery *gallery, QObject *parent) :
	QAbstractListModel(parent), m_gallery(gallery), m_count(-1), m_cache(1000)
{
}

void ThumbnailModel::setQuery(SpecialQuery query)
{
	QString sql;
	switch(query) {
	case QUERY_ALL:
		sql = "SELECT * FROM picture WHERE hidden=0 ORDER BY picid ASC";
		break;
	case QUERY_UNTAGGED:
		sql = "SELECT * FROM picture WHERE hidden=0 AND tags=\"\" ORDER BY picid DESC";
		break;
	case QUERY_NEW:
		sql = "SELECT * FROM picture WHERE hidden=0 ORDER BY picid DESC LIMIT 100";
		break;
	case QUERY_HIDDEN:
		sql = "SELECT * FROM picture WHERE hidden=1 ORDER BY picid ASC";
		break;
	default:
		qFatal("Unhandled query mode");
		return;
	}

	beginResetModel();

	m_count = -1;
	m_cache.clear();

	QSqlQuery q(m_gallery->database()->get());
	if(!q.exec("DROP VIEW IF EXISTS t_picview"))
		qDebug() << "Couldn't drop old t_picview:" << q.lastError().text();

	if(!q.exec("CREATE TEMP VIEW t_picview AS " + sql))
		qDebug() << "Couldn't create new t_picivew:" << q.lastError().text();

	endResetModel();
}

void ThumbnailModel::setQuery(const TagQuery &query)
{
	beginResetModel();

	m_count = -1;
	m_cache.clear();

	// Get the "shortlist" and filter it
	QSqlQuery q(m_gallery->database()->get());

	QVector<int> shortlist;
	q.setForwardOnly(true);
	q.exec("SELECT picid, tagid, tagset FROM tagmap WHERE picid IN (SELECT picid FROM tagmap WHERE tagid IN (" + query.mentionedTagIds().join(",") + ") GROUP BY picid) ORDER BY picid, tagset ASC");
	if(q.next()) {
		while(true) {
			TagIdSet tags = TagIdSet::getFromResults(q);
			if(tags.pictureId()<0)
				break;

			if(query.match(tags))
				shortlist.append(tags.pictureId());
		}
	}

	qDebug() << "shortlist" << shortlist;

	// Store the filtered list
	if(!q.exec("DROP TABLE IF EXISTS t_query"))
		qDebug() << "Couldn't drop old t_query:" << q.lastError().text();

	if(!q.exec("CREATE TEMP TABLE t_query (picid INTEGER NOT NULL PRIMARY KEY)"))
		qDebug() << "Couldn't create new t_query:" << q.lastError().text();

	q.prepare("INSERT INTO t_query VALUES (?)");
	foreach(int id, shortlist) {
		q.bindValue(0, id);
		q.exec();
	}

	// Select filtered list view
	if(!q.exec("DROP VIEW IF EXISTS t_picview"))
		qDebug() << "Couldn't drop old t_picview:" << q.lastError().text();

	if(!q.exec("CREATE TEMP VIEW t_picview AS SELECT * FROM picture JOIN t_query USING (picid) WHERE hidden=0 ORDER BY picid ASC"))
		qDebug() << "Couldn't create new t_picivew:" << q.lastError().text();

	endResetModel();
}


void ThumbnailModel::refreshQuery()
{
	beginResetModel();

	m_count = -1;
	m_cache.clear();

	endResetModel();
}

void ThumbnailModel::uncache(int index)
{
	m_cache.remove(index);
}

int ThumbnailModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	if(m_count<0) {
		QSqlQuery q("SELECT COUNT(picid) FROM t_picview", m_gallery->database()->get());
		if(q.next())
			m_count = q.value(0).toInt();
		else
			qDebug() << "Couldn't get picture count:" << q.lastError().text();
	}
	return m_count;
}

QVariant ThumbnailModel::data(const QModelIndex &index, int role) const
{
	if(index.row() >= 0 && index.row() < rowCount(index)) {
		const Picture *picture = pictureAt(index.row());
		if(picture==0)
			return QVariant();

		if(role==Qt::DisplayRole) {
			QString name = picture->title();
			if(name.length() > 8)
				name.truncate(8);
			return name;
		} else if(role==Qt::ToolTipRole)
			return picture->tagString();
		else if(role==Qt::DecorationRole)
			return IconCache::getInstance().get(m_gallery, *picture);
	}
	return QVariant();
}

const Picture *ThumbnailModel::pictureAt(int index) const {
	Picture *picture = m_cache[index];
	if(picture==0) {
		// If not found in cache, load some
		QSqlQuery q("SELECT picid, filename, hidden, title, tags, rotation FROM t_picview LIMIT 10 OFFSET " + QString::number(index), m_gallery->database()->get());

		int i = index;
		while(q.next()) {
			Picture *p = new Picture(q.value(0).toInt(), q.value(1).toString(), q.value(2).toBool(), q.value(3).toString(), q.value(4).toString(), q.value(5).toInt());
			m_cache.insert(i++, p);
			if(picture==0)
				picture=p;
		}
		if(picture==0)
			qDebug() << "Couldn't get picture:" << q.lastError().text();
	}

	return picture;
}
