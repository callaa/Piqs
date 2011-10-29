#ifndef THUMBNAILMODEL_H
#define THUMBNAILMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QCache>

#include "picture.h"

class Gallery;

//! Item model for thumbnails
/**

  */
class ThumbnailModel : public QAbstractListModel
{
    Q_OBJECT
public:
	ThumbnailModel(const Gallery *gallery, QObject *parent = 0);

	int rowCount(const QModelIndex &parent) const;

	QVariant data(const QModelIndex &index, int role) const;

	//! Get the picture at the given index
	const Picture *pictureAt(int index) const;

signals:

public slots:

	//! Set the query string and filter the view
	void setQuery(const QString& query);

	void refreshQuery();

private:
	const Gallery *m_gallery;

	mutable int m_count;
	mutable QCache<int, Picture> m_cache;
};

#endif // THUMBNAILMODEL_H
