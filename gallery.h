#ifndef GALLERY_H
#define GALLERY_H

#include <QDir>
#include <QList>

#include "picture.h"
#include "database.h"

class Database;
class QSqlQuery;

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

	//! Get the total number of images in the gallery
	int totalCount() const;

	//! Get the gallery root directory
	/**
	  * Every picture in the gallery is found either in this directory
	  * or one of its subdirectories.
	  */
	const QDir& root() const { return m_root; }

	//! Get the gallery metadata directory
	const QDir& metadir() const { return m_metadir; }

	//! Get the database for this gallery
	const Database* database() const { return m_database; }

private:
	static QDir findRootGallery(QDir dir);

	void rescan(const QStringList& filefilter, const QString& prefix, const QDir& root, QSqlQuery& query);

	const QDir m_root;
	QDir m_metadir;
	Database *m_database;
};

#endif // GALLERY_H
