#include <QDebug>
#include <QSqlQuery>
#include <QVariant>

#include "picture.h"
#include "gallery.h"
#include "tagrules.h"

Picture::Picture()
	: m_id(0), m_relativename(QString()), m_hidden(false), m_title(QString()), m_tags(QString()), m_rotation(0)
{

}

Picture::Picture(int id, const QString& filename, bool hidden, const QString& title, const QString& tags, int rotation)
	: m_id(id), m_relativename(filename), m_hidden(hidden), m_title(title), m_tags(tags), m_rotation(rotation)
{
}

QString Picture::fullpath(const Gallery *gallery) const
{
	return gallery->root().absoluteFilePath(m_relativename);
}

void Picture::saveTitle(const Database *db, const QString &newtitle)
{
	m_title = newtitle;
	QSqlQuery q(db->get());
	q.prepare("UPDATE picture SET title=? WHERE picid=?");
	q.addBindValue(newtitle);
	q.addBindValue(m_id);
	if(!q.exec())
		qDebug() << "Couldn't save new title for picture" << m_id;
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
		qDebug() << "Couldn't save new tags to picture" << m_id;
	}

	TagIdSet tagset = TagIdSet(TagSet::parse(tags), db->tags(), m_id);

	TagImplications::load(db).apply(tagset);

	tagset.save(db);
}
