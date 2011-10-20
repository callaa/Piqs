#ifndef GALLERY_H
#define GALLERY_H

#include <QDir>
#include <QList>

#include "picture.h"

class Database;

class Gallery : public QObject
{
	Q_OBJECT
public:
	//! The directory where metadata and caches are stored
	/**
	 * This is the (hidden) subdirectory under the gallery root
	 * path.
	 */
	static const QString METADIR;

	//! Construct the gallery.
	/**
	 * The image database is opened. If it does not exist,
	 * it will be created.
	 */
	Gallery(const QDir& root, QObject *parent=0);

	~Gallery();

	//! Rescan the gallery directory tree for image files.
	/**
	 * Any images not already in the database will be added.
	 */
	void rescan();

	//! Get the gallery root directory
	/**
	  * Every picture in the gallery is found either in this directory
	  * or one of its subdirectories.
	  */
	const QDir& root() const { return _root; }

	//! Get the gallery metadata directory
	const QDir& metadir() const { return _metadir; }

	//! Get the number of images in the gallery
	int imageCount() const { return _pictures.size(); }

	//! Get a picture from the gallery
	const Picture& picture(int index) const { return _pictures.at(index); }

private:
	static QDir findRootGallery(QDir dir);

	void rescan(const QStringList& filefilter, const QString& prefix, const QDir& root);

	const QDir _root;
	QDir _metadir;
	QList<Picture> _pictures;
	Database *_database;
};

#endif // GALLERY_H
