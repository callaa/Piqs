#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>

#include "picture.h"

class QGraphicsScene;
class Gallery;

namespace Ui {
    class ImageView;
}

//! Single image viewer
class ImageView : public QWidget
{
    Q_OBJECT

public:
	explicit ImageView(const Gallery *gallery, QWidget *parent = 0);
    ~ImageView();

	//! Check if image autoscaling is enabled
	bool isAutofit() const;

	//! Set autofitting state
	void setAutofit(bool fit);

public slots:
	//! Select the picture to show
	void setPicture(const Picture& picture);

	void zoomin();
	void zoomout();
	void zoomfit(bool checked);
	void zoomorig();

signals:
	//! User wants to get out of this view
	void exitView();

	//! Request for the next picture in the browser to be shown
	void requestNext();

	//! Request for the previous picture in the browser to be shown
	void requestPrev();

protected:
	void keyPressEvent(QKeyEvent *e);
	void resizeEvent(QResizeEvent *e);

private:
	void scaleToFit();

	Ui::ImageView *m_ui;
	const Gallery *m_gallery;
	Picture m_picture;
	QGraphicsScene *m_scene;

};

#endif // IMAGEVIEW_H
