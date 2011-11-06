//
// This file is part of Piqs.
// 
// Piqs is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Piqs is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Piqs.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>

#include "picture.h"

class QGraphicsScene;
class QGraphicsItem;
class QMenu;
class Gallery;

namespace Ui {
    class ImageView;
}

//! Single image viewer
class ImageView : public QWidget
{
    Q_OBJECT

public:
	explicit ImageView(Gallery *gallery, QWidget *parent = 0);
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

	//! Save changes to title
	void saveTitle();

	//! Save changes to tags
	void saveTags();

	//! Show image info dialog
	void showInfo();

protected slots:
	void pictureContextMenu(const QPoint& point);
	void rotateLeft();
	void rotateRight();

signals:
	//! User wants to get out of this view
	void exitView();

	//! Request for the next picture in the browser to be shown
	void requestNext();

	//! Request for the previous picture in the browser to be shown
	void requestPrev();

	//! Currently open image has been changed and should be removed from the cache
	void changed();

protected:
	void keyPressEvent(QKeyEvent *e);
	void resizeEvent(QResizeEvent *e);

private:
	void adjustSceneRect(QGraphicsItem *item);
	void scaleToFit();

	QAction *m_autofit;

	Ui::ImageView *m_ui;
	Gallery *m_gallery;
	Picture m_picture;
	QGraphicsScene *m_scene;
	QMenu *m_imgctxmenu;
};

#endif // IMAGEVIEW_H
