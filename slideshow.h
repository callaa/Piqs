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
#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <QGraphicsView>

class ThumbnailModel;
class Gallery;

class QItemSelectionModel;
class QGraphicsView;
class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsTextItem;
class QGraphicsItem;
class QTimer;

class Slideshow : public QGraphicsView
{
    Q_OBJECT
public:
	Slideshow(Gallery *gallery, ThumbnailModel *model, QVector<int> selection = QVector<int>(), QWidget *parent = 0);
	~Slideshow();

	void start();

signals:

public slots:
	void prevSlide();
	void nextSlide();
	void jumpSlide(int jump);
	void togglePause();

protected:
	void resizeEvent(QResizeEvent *e);
	void keyPressEvent(QKeyEvent *e);

private:
	void setTimerText();
	void addShadowEffect(QGraphicsItem *item) const;
	qreal calcScale(const QSizeF& picture, const QSizeF& screen) const;

	Gallery *m_gallery;
	ThumbnailModel *m_model;
	QVector<int> m_selection;
	QGraphicsScene *m_scene;

	QGraphicsPixmapItem *m_picture;
	QGraphicsItem *m_pauseglyph;
	QGraphicsTextItem *m_timertext;
	int m_pos;
	bool m_paused;

	bool m_scalefill;
	bool m_upscale;

	QTimer *m_slidetimer;
};

#endif // SLIDESHOW_H
