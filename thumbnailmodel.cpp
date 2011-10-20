#include <QDir>
#include <QDebug>

#include "thumbnailmodel.h"
#include "gallery.h"

ThumbnailModel::ThumbnailModel(const Gallery *gallery, QObject *parent) :
	QAbstractListModel(parent), _gallery(gallery)
{
}

int ThumbnailModel::rowCount(const QModelIndex &parent) const
{
	return _gallery->imageCount();
}

QVariant ThumbnailModel::data(const QModelIndex &index, int role) const
{
	if(index.row() >= 0 && index.row() <= _gallery->imageCount()) {
		const Picture& picture = _gallery->picture(index.row());
		if(role==Qt::DisplayRole) {
			QString name = picture.name();
			if(name.length() > 8)
				name.truncate(8);
			return name;
		} else if(role==Qt::ToolTipRole)
			return picture.name();
		else if(role==Qt::DecorationRole)
			return picture.icon();
	}
	return QVariant();
}

