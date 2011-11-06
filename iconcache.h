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
	 \brief Get an icon for the image from the gallery.

	 If an icon is not available right away, a placeholder
	 icon will be returned and icon loading will be queued in the
	 background.
	 @param gallery the gallery from which to get the icon
	 @param picture the picture whose thumbnail to get
	 @return icon
	 */
	QPixmap get(const Gallery* gallery, const Picture& picture);

	//! Delete a thumbnail
	void remove(const Gallery *gallery, const Picture& picture);

private:
    IconCache();
	IconCache(const IconCache& ic);
	IconCache& operator=(const IconCache& ic);

	//! Generate a thumbnail image and save it to the file system
	void cacheImage(const QString &imagefile, const QString& cachefile);

	//! Cached icon pixmaps
	QCache<QString, QPixmap> m_cache;

	//! A pixmap that is shown when the true icon is not yet available
	QPixmap m_placeholder;

	//! Set of thumbnails queued
	QSet<QString> m_loading;

	//! Lock for _loading
	QMutex m_lock;

	static const int ICON_SIZE = 128;
};

#endif // ICONCACHE_H
