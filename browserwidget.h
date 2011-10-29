#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H

#include <QWidget>

class QListView;
class QLineEdit;
class QModelIndex;

class Gallery;
class Picture;
class ThumbnailModel;

//! Image thumbnail browser widget
class BrowserWidget : public QWidget
{
    Q_OBJECT
public:
	explicit BrowserWidget(Gallery *gallery, QWidget *parent = 0);

	//! Get the picture at the specific index
	const Picture *getPictureAt(int index) const;

	//! Get the number of pictures in this view
	int getPictureCount() const;

	//! Get the index of the currently selected image
	int getCurrentSelection() const;

	//! Set the selection
	void selectPicture(int index);

signals:
	//! User selected a picture for closer viewing
	void pictureSelected(const Picture& picture);

public slots:
	void openPicture(const QModelIndex& index);
	void refreshQuery();

private:
	Gallery *m_gallery;
	QListView *m_view;
	QLineEdit *m_searchbox;
	ThumbnailModel *m_model;
};

#endif // BROWSERWIDGET_H
