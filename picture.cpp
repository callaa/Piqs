#include <QDebug>
#include "gallery.h"
#include "picture.h"
#include "iconcache.h"

Picture::Picture(const Gallery *gallery, const QString& relativename, const QFileInfo& file)
	: _gallery(gallery), _relativename(relativename), _file(file)
{
}

QPixmap Picture::icon() const
{
	return IconCache::getInstance().get(_gallery, _relativename);
}
