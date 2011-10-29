#include "picture.h"
#include "gallery.h"

Picture::Picture()
	: m_id(0), m_relativename(QString()), m_hidden(false), m_title(QString()), m_tags(QString())
{

}

Picture::Picture(int id, const QString& filename, bool hidden, const QString& title, const QString& tags)
	: m_id(id), m_relativename(filename), m_hidden(hidden), m_title(title), m_tags(tags)
{
}

QString Picture::fullpath(const Gallery *gallery) const
{
	return gallery->root().absoluteFilePath(m_relativename);
}
