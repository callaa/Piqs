#include <QtConcurrentRun>
#include <QPainter>

#include "iconcache.h"
#include "gallery.h"
#include "picture.h"

IconCache::IconCache()
	: _cache(255), _placeholder(QPixmap(ICON_SIZE, ICON_SIZE))
{
	_placeholder.fill(Qt::gray);
}

IconCache& IconCache::getInstance()
{
	static IconCache *singleton = new IconCache();
	return *singleton;
}

QPixmap IconCache::get(const Gallery *gallery, const Picture &picture)
{
	// The thumbnail files are distributed into subdirectories
	const QString cachefile = gallery->metadir().absoluteFilePath(
				QString::number(picture.id()/10000) + QDir::separator() + QString::number(picture.id()/100) + QDir::separator() +
				QString::number(picture.id()) + ".jpg");

	QPixmap *icon = _cache[cachefile];

	if(icon!=0) {
		// Was cached
		return *icon;
	} else {
		// Not cached, try loading from filesystem
		icon = new QPixmap(cachefile);
		if(!icon->isNull()) {
			// Found filesystem cache
			_cache.insert(cachefile, icon);

			return *icon;
		} else {
			// No filesystem cache. Queue generation.
			delete icon;
			_lock.lock();
			if(!_loading.contains(cachefile)) {
				_loading.insert(cachefile);
				_lock.unlock();

				QtConcurrent::run(this, &IconCache::cacheImage, gallery, picture.relativeName(), cachefile);
			} else {
				_lock.unlock();;
			}

			return _placeholder;
		}
	}
}

void IconCache::cacheImage(const Gallery *gallery, const QString &image, const QString& cachefile)
{
	QString src = gallery->root().absoluteFilePath(image);
	QImage img(src);
	QImage icon;
	if(icon.width() > ICON_SIZE || icon.height() > ICON_SIZE) {
		icon = img.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	} else {
		icon = QImage(ICON_SIZE, ICON_SIZE, QImage::Format_RGB32);
		QPainter painter(&icon);
		painter.fillRect(0, 0, ICON_SIZE, ICON_SIZE, Qt::white);
		painter.drawImage(ICON_SIZE/2 - img.width()/2, ICON_SIZE/2 - img.height()/2, img);
	}

	gallery->metadir().mkpath(QFileInfo(cachefile).dir().path());

	icon.save(cachefile);
	_lock.lock();
	_loading.remove(cachefile);
	_lock.unlock();
}
