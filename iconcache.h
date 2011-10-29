#ifndef ICONCACHE_H
#define ICONCACHE_H

#include <QPixmap>
#include <QCache>
#include <QSet>
#include <QMutex>

class Gallery;
class Picture;

//! Memory cache and thumbnail generation
class IconCache
{
public:
	static IconCache& getInstance();

	/**
	 Get an icon for the image from the gallery.
	 If an icon is not available right away, a placeholder
	 icon will be returned and icon loading will be queued in the
	 background.
	 @param gallery the gallery from which to get the icon
	 @param picture the picture whose thumbnail to get
	 @return icon
	 */
	QPixmap get(const Gallery* gallery, const Picture& picture);

private:
    IconCache();
	IconCache(const IconCache& ic);
	IconCache& operator=(const IconCache& ic);

	//! Generate a thumbnail image and save it to the file system
	void cacheImage(const Gallery *gallery, const QString &image, const QString& cachefile);

	//! Cached icon pixmaps
	QCache<QString, QPixmap> _cache;

	//! A pixmap that is shown when the true icon is not yet available
	QPixmap _placeholder;

	//! Set of thumbnails queued
	QSet<QString> _loading;

	//! Lock for _loading
	QMutex _lock;

	static const int ICON_SIZE = 128;
};

#endif // ICONCACHE_H
