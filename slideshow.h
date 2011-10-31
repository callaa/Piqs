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
	void addShadowEffect(QGraphicsItem *item);

	Gallery *m_gallery;
	ThumbnailModel *m_model;
	QVector<int> m_selection;
	QGraphicsScene *m_scene;

	QGraphicsPixmapItem *m_picture;
	QGraphicsItem *m_pauseglyph;
	QGraphicsTextItem *m_timertext;
	int m_pos;
	bool m_paused;

	QTimer *m_slidetimer;
};

#endif // SLIDESHOW_H
