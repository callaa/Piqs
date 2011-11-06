#ifndef THUMBNAILMODEL_H
#define THUMBNAILMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QCache>

#include "picture.h"

class Gallery;
class TagQuery;

//! Item model for thumbnails
/**

  */
class ThumbnailModel : public QAbstractListModel
{
    Q_OBJECT
public:
	//! Special query modes
	enum SpecialQuery { QUERY_ALL, QUERY_UNTAGGED, QUERY_NEW, QUERY_HIDDEN, QUERY_MISSING, QUERY_DUPLICATE, QUERY_FILENAME, QUERY_TITLE, QUERY_HASH };

	ThumbnailModel(const Gallery *gallery, QObject *parent = 0);

	int rowCount(const QModelIndex &parent) const;

	QVariant data(const QModelIndex &index, int role) const;

	Qt::ItemFlags flags(const QModelIndex& index) const;

	QMimeData *mimeData(const QModelIndexList &indexes) const;

	//! Get the picture at the given index
	const Picture *pictureAt(int index) const;

	/**
	  \brief Get a list of pictures

	  Use this method for making changes that may alter the underlying view, e.g.
	  changing visibility, tags, deleting, etc.
	  Remember to call refreshQuery() afterwards!
	  */
	QList<Picture> pictures(const QModelIndexList& list);

	//! Remove the picture at index from the cache
	void uncache(int index, bool removed=false);

	//! Set a special (non-tag based) query and filter the view
	void setQuery(SpecialQuery query, const QString& param=QString());

	//! Set the query string and filter the view
	void setQuery(const TagQuery& query);

signals:

public slots:
	void refreshQuery();

private:
	const Gallery *m_gallery;

	mutable int m_count;
	mutable QCache<int, Picture> m_cache;
};

#endif // THUMBNAILMODEL_H
