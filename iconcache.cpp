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
#include <QtConcurrentRun>
#include <QPainter>

#include "iconcache.h"
#include "gallery.h"
#include "picture.h"

IconCache::IconCache()
	: m_cache(255), m_placeholder(QPixmap(ICON_SIZE, ICON_SIZE))
{
	m_placeholder.fill(Qt::gray);
}

IconCache& IconCache::getInstance()
{
	static IconCache *singleton = new IconCache();
	return *singleton;
}

static QString cachefilepath(const Gallery *gallery, const Picture &picture) {
	if(picture.hash().isEmpty())
		return QString();

	return gallery->metadir().absoluteFilePath(picture.hash().left(2) + QDir::separator() + picture.hash() + ".png");
}

QPixmap IconCache::get(const Gallery *gallery, const Picture &picture)
{
	const QString cachefile = cachefilepath(gallery, picture);

	if(cachefile.isEmpty())
		return m_placeholder;

	QPixmap *icon = m_cache[cachefile];

	if(icon!=0) {
		// Was cached
		return *icon;
	} else {
		// Not cached, try loading from filesystem
		icon = new QPixmap(cachefile);
		if(!icon->isNull()) {
			// Found filesystem cache
			m_cache.insert(cachefile, icon);

			return *icon;
		} else {
			// No filesystem cache. Queue generation.
			delete icon;
			m_lock.lock();
			if(!m_loading.contains(cachefile)) {
				m_loading.insert(cachefile);
				m_lock.unlock();

				QtConcurrent::run(this, &IconCache::cacheImage, picture.fullpath(gallery), cachefile);
			} else {
				m_lock.unlock();;
			}

			return m_placeholder;
		}
	}
}

void IconCache::remove(const Gallery *gallery, const Picture& picture)
{
	const QString cachefile = cachefilepath(gallery, picture);
	QFile(cachefile).remove();
	m_cache.remove(cachefile);
}

void IconCache::cacheImage(const QString &imagefile, const QString& cachefile)
{
	QImage img(imagefile);
	QImage icon;

	// If source image is larger than thumbnail size (as is usual),
	// scale down
	if(img.width() > ICON_SIZE || img.height() > ICON_SIZE) {
		img = img.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	// Square thumbnail if not already
	if(img.width() != ICON_SIZE || img.height() != ICON_SIZE) {
		icon = QImage(ICON_SIZE, ICON_SIZE, QImage::Format_ARGB32);
		icon.fill(0);
		QPainter painter(&icon);
		painter.drawImage(ICON_SIZE/2 - img.width()/2, ICON_SIZE/2 - img.height()/2, img);
	} else {
		icon = img;
	}



	if(!icon.save(cachefile)) {
		// Save failed? Maybe directory is missing. Try again
		QFileInfo(cachefile).dir().mkpath(".");
		if(!icon.save(cachefile)) {
			qWarning("Couldn't save thumbnail: %s", cachefile.toLocal8Bit().constData());
		}
	}

	m_lock.lock();
	m_loading.remove(cachefile);
	m_lock.unlock();
}
