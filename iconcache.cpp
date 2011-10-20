#include <QtConcurrentRun>
#include "iconcache.h"
#include "gallery.h"

IconCache::IconCache()
	: _placeholder(QPixmap(ICON_SIZE, ICON_SIZE))
{
	_placeholder.fill(Qt::gray);
}

IconCache& IconCache::getInstance()
{
	static IconCache singleton;
	return singleton;
}

QPixmap IconCache::get(const Gallery *gallery, const QString &image)
{
	const QString cachefile = gallery->metadir().absoluteFilePath(image) + ".jpg";

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

				QtConcurrent::run(this, &IconCache::cacheImage, gallery, image, cachefile);
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
	QImage icon = QImage(src).scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	gallery->metadir().mkpath(QFileInfo(image).dir().path());

	icon.save(cachefile);
	_lock.lock();
	_loading.remove(cachefile);
	_lock.unlock();
}
