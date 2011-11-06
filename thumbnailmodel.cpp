//
// This file is part of Piqs.
// 
// Piqs is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Piqs is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Piqs.  If not, see <http://www.gnu.org/licenses/>.
//
#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QMimeData>
#include <QUrl>

#include "thumbnailmodel.h"
#include "iconcache.h"
#include "gallery.h"
#include "tagquery.h"

ThumbnailModel::ThumbnailModel(const Gallery *gallery, QObject *parent) :
	QAbstractListModel(parent), m_gallery(gallery), m_count(-1), m_cache(1000)
{
}

void ThumbnailModel::setQuery(SpecialQuery query, const QString &param)
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
		sql = QString("SELECT * FROM picture WHERE hidden=0 AND picid>%1 ORDER BY picid DESC").arg(m_gallery->database()->getSetting("lastnewid").toInt());
		break;
	case QUERY_HIDDEN:
		sql = "SELECT * FROM picture WHERE hidden=1 ORDER BY picid ASC";
		break;
	case QUERY_MISSING:
		sql = "SELECT * FROM picture WHERE found=0 ORDER BY picid ASC";
		break;
	case QUERY_DUPLICATE:
		sql = "SELECT * FROM picture JOIN duplicate USING (picid) ORDER BY picid ASC";
		break;
	case QUERY_FILENAME:
		sql = "SELECT * FROM picture WHERE filename GLOB " + m_gallery->database()->esc("*" + param + "*") + " ORDER BY picid ASC";
		break;
	case QUERY_TITLE:
		sql = "SELECT * FROM picture WHERE title LIKE " + m_gallery->database()->esc("%" + param + "%") + " ORDER BY picid ASC";
		break;
	case QUERY_HASH:
		sql = "SELECT * FROM picture WHERE hash LIKE " + m_gallery->database()->esc(param + "%") + " ORDER BY picid ASC";
		break;
	default:
		qFatal("Unhandled query mode");
		return;
	}

	beginResetModel();

	m_count = -1;
	m_cache.clear();

	// Drop old query view
	QSqlQuery q(m_gallery->database()->get());
	if(!q.exec("DROP VIEW IF EXISTS t_picview"))
		Database::showError("Couldn't drop old t_picview", q);

	// Create new query view visible to this connection only
	if(!q.exec("CREATE TEMP VIEW t_picview AS " + sql))
		Database::showError("Couldn't create new t_picview", q);

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

	// Store the filtered list
	if(!q.exec("DROP TABLE IF EXISTS t_query"))
		Database::showError("Couldn't drop old t_query", q);

	if(!q.exec("CREATE TEMP TABLE t_query (picid INTEGER NOT NULL PRIMARY KEY)"))
		Database::showError("Couldn't create new t_query", q);

	q.prepare("INSERT INTO t_query VALUES (?)");
	foreach(int id, shortlist) {
		q.bindValue(0, id);
		q.exec();
	}

	// Select filtered list view
	if(!q.exec("DROP VIEW IF EXISTS t_picview"))
		Database::showError("Couldn't drop old t_picview", q);

	if(!q.exec("CREATE TEMP VIEW t_picview AS SELECT * FROM picture JOIN t_query USING (picid) WHERE hidden=0 ORDER BY picid ASC"))
		Database::showError("Couldn't create new t_picview", q);

	endResetModel();
}


void ThumbnailModel::refreshQuery()
{
	beginResetModel();

	m_count = -1;
	m_cache.clear();

	endResetModel();
}

void ThumbnailModel::uncache(int index, bool removed)
{

	if(removed) {
		this->beginRemoveRows(QModelIndex(), index, index);
		m_cache.clear();
		m_count--;
		this->beginRemoveRows(QModelIndex(), index, index);
	} else {
		m_cache.remove(index);
	}

}

int ThumbnailModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	if(m_count<0) {
		QSqlQuery q("SELECT COUNT(picid) FROM t_picview", m_gallery->database()->get());
		if(q.next())
			m_count = q.value(0).toInt();
		else
			qDebug() << "Couldn't get picture count!";
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
			//if(name.length() > 8)
			//	name.truncate(8);
			return name;
		} else if(role==Qt::ToolTipRole)
			return picture->tagString();
		else if(role==Qt::DecorationRole)
			return IconCache::getInstance().get(m_gallery, *picture);
	}
	return QVariant();
}

Qt::ItemFlags ThumbnailModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
	if (index.isValid())
		return Qt::ItemIsDragEnabled | defaultFlags;
	else
		return defaultFlags;
}

QMimeData *ThumbnailModel::mimeData(const QModelIndexList &indexes) const
{
	QList<QUrl> imgs;
	foreach (const QModelIndex &index, indexes) {
		if (index.isValid()) {
			imgs << QUrl::fromLocalFile(pictureAt(index.row())->fullpath(m_gallery));
		}
	}
	if(!imgs.isEmpty()) {
		QMimeData *mime = new QMimeData();
		mime->setUrls(imgs);
		return mime;
	}
	return 0;
}

const Picture *ThumbnailModel::pictureAt(int index) const {
	Picture *picture = m_cache[index];
	if(picture==0) {
		// If not found in cache, load some
		QSqlQuery q("SELECT picid, filename, hidden, title, tags, rotation, hash FROM t_picview LIMIT 10 OFFSET " + QString::number(index), m_gallery->database()->get());

		int i = index;
		while(q.next()) {
			Picture *p = new Picture(q.value(0).toInt(), q.value(1).toString(), q.value(2).toBool(), q.value(3).toString(), q.value(4).toString(), q.value(5).toInt(), q.value(6).toString());
			m_cache.insert(i++, p);
			if(picture==0)
				picture=p;
		}
		if(picture==0)
			qDebug() << "Couldn't get picture at index" << index;
	}

	return picture;
}

QList<Picture> ThumbnailModel::pictures(const QModelIndexList& list)
{
	QList<Picture> pictures;
	for(int i=0;i<list.count();++i)
		pictures << *pictureAt(list.at(i).row());
	return pictures;
}
