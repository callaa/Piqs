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
	return gallery->metadir().absoluteFilePath(
				QString::number(picture.id()/10000) + QDir::separator() + QString::number(picture.id()/100) + QDir::separator() +
				QString::number(picture.id()) + ".jpg");;
}

QPixmap IconCache::get(const Gallery *gallery, const Picture &picture)
{
	const QString cachefile = cachefilepath(gallery, picture);

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

				QtConcurrent::run(this, &IconCache::cacheImage, gallery, picture.relativeName(), cachefile);
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

void IconCache::cacheImage(const Gallery *gallery, const QString &image, const QString& cachefile)
{
	QString src = gallery->root().absoluteFilePath(image);
	QImage img(src);
	QImage icon;
	if(img.width() > ICON_SIZE || img.height() > ICON_SIZE) {
		icon = img.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	} else {
		icon = QImage(ICON_SIZE, ICON_SIZE, QImage::Format_RGB32);
		QPainter painter(&icon);
		painter.fillRect(0, 0, ICON_SIZE, ICON_SIZE, Qt::white);
		painter.drawImage(ICON_SIZE/2 - img.width()/2, ICON_SIZE/2 - img.height()/2, img);
	}

	gallery->metadir().mkpath(QFileInfo(cachefile).dir().path());

	icon.save(cachefile);
	m_lock.lock();
	m_loading.remove(cachefile);
	m_lock.unlock();
}
