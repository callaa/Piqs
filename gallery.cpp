#include <QStringList>
#include <QFileInfoList>
#include <QDebug>
#include <QImageReader>

#include "gallery.h"
#include "database.h"

const QString Gallery::METADIR = ".piqs";

Gallery::Gallery(const QDir& root, QObject *parent)
	: QObject(parent), _root(findRootGallery(root))
{
	// TODO error handling
	if(!_root.exists(METADIR))
		_root.mkdir(METADIR);
	_metadir = _root;
	_metadir.cd(METADIR);

	_database = new Database(_metadir, this);
}

/**
 * Travel up the directory stack until a parent directory containing
 * a .piqs directory is found.
 * <p>If no such directory is found, the original directory is returned.
 */
QDir Gallery::findRootGallery(QDir dir)
{
	QDir top = dir;
	while(1) {
		qDebug() << "finding root gallery" << dir;
		if(dir.exists(METADIR))
			return dir;
		else if(!dir.cdUp())
			break;
	}
	return top;
}


Gallery::~Gallery()
{
}

void Gallery::rescan()
{
	_pictures.clear();

	// Get a list of image formats supported by Qt
	QStringList filefilter;
	foreach(QByteArray format, QImageReader::supportedImageFormats()) {
		QString fmt = (QString("*.") + format).toLower();
		if(!filefilter.contains(fmt))
			filefilter << fmt;
	}

	// Recursively scan from gallery root directory up
	rescan(filefilter, QString(), _root);

	_database->syncPictures(_pictures);
}

void Gallery::rescan(const QStringList& filefilter, const QString& prefix, const QDir& root)
{
	// Recursively scan subdirectories
	QStringList dirs = root.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach(const QString& name, dirs) {
		rescan(filefilter, prefix + name + "/", QDir(root.absoluteFilePath(name)));
	}

	// Scan supported images
	QFileInfoList files = root.entryInfoList(filefilter, QDir::Files | QDir::Readable);
	foreach(const QFileInfo& file, files) {
		_pictures.append(Picture(this, prefix + file.fileName(), file));
	}

}
