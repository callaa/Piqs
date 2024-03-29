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
#include <QDebug>
#include <QSqlQuery>
#include <QVariant>

#include "picture.h"
#include "gallery.h"
#include "tagrules.h"
#include "iconcache.h"

Picture::Picture()
	: m_id(0), m_relativename(QString()), m_hidden(false), m_title(QString()), m_tags(QString()), m_rotation(0), m_hash(QString())
{

}

Picture::Picture(int id, const QString& filename, bool hidden, const QString& title, const QString& tags, int rotation, const QString& hash)
	: m_id(id), m_relativename(filename), m_hidden(hidden), m_title(title), m_tags(tags), m_rotation(rotation), m_hash(hash)
{
}

QString Picture::fileName() const
{
	int sep = m_relativename.lastIndexOf(QDir::separator());
	if(sep<0)
		return m_relativename;
	else
		return m_relativename.mid(sep+1);
}

QString Picture::fullpath(const Gallery *gallery) const
{
	return gallery->root().absoluteFilePath(m_relativename);
}

void Picture::deleteFile(Gallery *gallery)
{
	QSqlQuery q(gallery->database()->get());
	if(!q.exec("DELETE FROM picture WHERE picid=" + QString::number(m_id))) {
		Database::showError("Couldn't delete file!", q);
		return;
	}

	QFile(fullpath(gallery)).remove();

	IconCache::getInstance().remove(gallery, *this);
}

void Picture::setHidden(Database *db, bool hidden)
{
	m_hidden = hidden;
	QSqlQuery q(db->get());
	if(!q.exec(QString("UPDATE picture SET hidden=%1 WHERE picid=%2").arg(m_hidden?1:0).arg(m_id)))
		Database::showError("Couldn't " + QString(hidden?"hide":"show") + " picture", q);
}

void Picture::setRotation(const Database *db, int rotation)
{
	m_rotation = rotation;
	QSqlQuery q(db->get());
	if(!q.exec(QString("UPDATE picture SET rotation=%1 WHERE picid=%2").arg(m_rotation).arg(m_id)))
		Database::showError("Couldn't change rotation", q);
}

void Picture::saveTitle(const Database *db, const QString &newtitle)
{
	m_title = newtitle;
	QSqlQuery q(db->get());
	q.prepare("UPDATE picture SET title=? WHERE picid=?");
	q.addBindValue(newtitle);
	q.addBindValue(m_id);
	if(!q.exec())
		Database::showError("Couldn't save new title for picture", q);
}

/**
  The original tag string is saved as is. The inferred and normalized tags will be saved in the database as well.
  \param db the database to use
  \param tags new tag string
  \return saved set of tags
  */
void Picture::saveTags(Database *db, const QString& tags)
{
	m_tags = tags;
	QSqlQuery q(db->get());
	q.prepare("UPDATE picture SET tags=? WHERE picid=?");
	q.addBindValue(tags);
	q.addBindValue(m_id);
	if(!q.exec()) {
		Database::showError("Couldn't save new tags for picture", q);
		return;
	}

	TagIdSet tagset = TagIdSet(TagSet::parse(tags), db->tags(), m_id);

	TagImplications::load(db).apply(tagset);

	tagset.save(db);
}
