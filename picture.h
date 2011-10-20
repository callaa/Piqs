#ifndef PICTURE_H
#define PICTURE_H

#include <QFileInfo>
#include <QPixmap>

class Gallery;

class Picture
{
public:
	Picture(const Gallery *gallery, const QString& relativename, const QFileInfo& file);

	//! Get the filename relative to gallery root
	QString relativeName() const { return _relativename; }

	QString name() const { return _file.fileName(); }

	QPixmap icon() const;

private:
	const Gallery *_gallery;
	QString _relativename;
	QFileInfo _file;
};

#endif // PICTURE_H
