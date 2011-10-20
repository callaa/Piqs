#ifndef THUMBNAILMODEL_H
#define THUMBNAILMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class Gallery;

class ThumbnailModel : public QAbstractListModel
{
    Q_OBJECT
public:
	ThumbnailModel(const Gallery *gallery, QObject *parent = 0);

	int rowCount(const QModelIndex &parent) const;

	QVariant data(const QModelIndex &index, int role) const;

signals:

public slots:

private:
	const Gallery *_gallery;
};

#endif // THUMBNAILMODEL_H
