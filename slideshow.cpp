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
#include <QDebug>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QResizeEvent>
#include <QDesktopWidget>

#include "slideshow.h"
#include "thumbnailmodel.h"
#include "gallery.h"

Slideshow::Slideshow(Gallery *gallery, ThumbnailModel *model, QVector<int> selection, QWidget *parent) :
	QGraphicsView(parent), m_gallery(gallery), m_model(model), m_selection(selection), m_picture(0)
{
	setWindowTitle(tr("Slideshow"));
	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowFlags(windowFlags() | Qt::Window);
	setFrameShape(QFrame::NoFrame);
	setFocusPolicy(Qt::StrongFocus);

	m_scene = new QGraphicsScene();
	m_scene->setBackgroundBrush(Qt::black);

	setScene(m_scene);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setInteractive(false);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	m_slidetimer = new QTimer(this);
	connect(m_slidetimer, SIGNAL(timeout()), this, SLOT(nextSlide()));

	// Construct the pause glyph
	QGraphicsLineItem *p1 = new QGraphicsLineItem(QLineF(20, 20, 20, 70));
	QGraphicsLineItem *p2 = new QGraphicsLineItem(QLineF(60, 20, 60, 70));
	QPen pen;
	pen.setWidth(15);
	pen.setCapStyle(Qt::RoundCap);
	pen.setColor(Qt::white);
	p1->setPen(pen);
	p2->setPen(pen);

	QGraphicsItemGroup *pauseglyph = new QGraphicsItemGroup();
	pauseglyph->setZValue(100);
	pauseglyph->addToGroup(p1);
	pauseglyph->addToGroup(p2);
	addShadowEffect(pauseglyph);

	pauseglyph->hide();
	m_pauseglyph = pauseglyph;
	m_scene->addItem(m_pauseglyph);

	// Construct timer text item
	m_timertext = new QGraphicsTextItem();
	m_timertext->setZValue(100);
	QFont font;
	font.setPixelSize(20);
	m_timertext->setFont(font);
	m_timertext->setDefaultTextColor(Qt::white);
	setTimerText();
	addShadowEffect(m_timertext);

	m_timertext->hide();
	m_scene->addItem(m_timertext);

	m_scalefill = gallery->database()->getSetting("slideshow.scale").toString() == "fill";
	m_upscale = gallery->database()->getSetting("slideshow.upscale").toBool();
}

Slideshow::~Slideshow()
{
}

void Slideshow::addShadowEffect(QGraphicsItem *item) const
{
	QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();

	shadow->setColor(Qt::black);
	shadow->setBlurRadius(5);
	shadow->setOffset(0, 0);
	item->setGraphicsEffect(shadow);
}

void Slideshow::setTimerText()
{
	m_timertext->setPlainText(QString(tr("%1 s")).arg(m_slidetimer->interval()/1000.0, 0, 'f', 1));
}

void Slideshow::resizeEvent(QResizeEvent *e)
{
	QGraphicsView::resizeEvent(e);

	qreal w = e->size().width();
	qreal h = e->size().height();
	m_scene->setSceneRect(-w/2.0, -h/2.0, w, h);

	centerOn(0, 0);

	m_pauseglyph->setPos(-w/2, -h/2);
	m_timertext->setPos(-w/2 + 10, -h/2 + 80);
}

void Slideshow::keyPressEvent(QKeyEvent *e)
{
	switch(e->key()) {
	case Qt::Key_Return:
	case Qt::Key_Enter:
		if(e->modifiers() & Qt::AltModifier) {
			if(this->isFullScreen()) {
				this->setWindowState(Qt::WindowMaximized);
				this->setGeometry(QDesktopWidget().screenGeometry(this).adjusted(50,50,-50,-50));
			} else
				this->setWindowState(Qt::WindowFullScreen);
		}
		break;
	case Qt::Key_Space:
		togglePause();
		break;
	case Qt::Key_Left:
		if(!m_paused)
			m_slidetimer->start();
		prevSlide();
		break;
	case Qt::Key_Right:
		if(!m_paused)
			m_slidetimer->start();
		nextSlide();
		break;
	case Qt::Key_Escape:
		this->close();
		break;
	case Qt::Key_Plus: {
		int i = m_slidetimer->interval() + 500;
		m_slidetimer->setInterval(i);
		setTimerText();
		m_timertext->show();
		break;
		}
	case Qt::Key_Minus: {
		int i = m_slidetimer->interval() - 500;
		if(i>=1000) {
			m_slidetimer->setInterval(i);
			setTimerText();
		}
		m_timertext->show();
		break;
		}
	default:
		QGraphicsView::keyPressEvent(e);
	}
}

void Slideshow::start()
{
	m_paused = false;

	this->show();
	// Note. If we start in fullscreen mode, the window won't get focus properly.
	//this->setWindowState(Qt::WindowFullScreen);
	this->setGeometry(QDesktopWidget().screenGeometry(this));

	this->setWindowState(Qt::WindowMaximized);
	this->raise();
	this->activateWindow();

	m_pos = -1;
	nextSlide();

	bool ok;
	double interval = this->m_gallery->database()->getSetting("slideshow.delay").toDouble(&ok);
	if(!ok)
		interval = 10;

	m_slidetimer->setInterval(interval * 1000);
	m_slidetimer->start();
}

void Slideshow::togglePause()
{
	if(m_paused) {
		m_slidetimer->start();
		m_pauseglyph->hide();
	} else {
		m_slidetimer->stop();
		m_pauseglyph->show();
	}
	m_paused = !m_paused;
}

qreal Slideshow::calcScale(const QSizeF& picture, const QSizeF& screen) const {
	qreal pa = picture.height() / picture.width();
	qreal sa = screen.height() / screen.width();

	qreal scale;
	if(m_scalefill ^ (pa > sa))
		scale = screen.height() / picture.height();
	else
		scale = screen.width() / picture.width();

	if(!m_upscale && scale > 1)
		scale = 1;

	return scale;
}

void Slideshow::prevSlide()
{
	jumpSlide(-1);
}

void Slideshow::nextSlide()
{
	jumpSlide(1);
}

void Slideshow::jumpSlide(int skip)
{
	const int count = m_selection.count()>1 ? m_selection.count() : m_model->rowCount(QModelIndex());
	m_pos = (m_pos + skip) % count;
	if(m_pos < 0)
		m_pos = count - m_pos;

	m_timertext->hide();

	qDebug() << "next slide" << m_pos << "of" << count;

	QRectF view = m_scene->sceneRect();

	int realpos;
	if(m_selection.count()>1)
		realpos = m_selection.at(m_pos);
	else
		realpos = m_pos;

	const Picture *pic = m_model->pictureAt(realpos);
	QGraphicsPixmapItem *newpic = new QGraphicsPixmapItem(pic->fullpath(m_gallery));
	QRectF bounds = newpic->boundingRect();

	newpic->setTransformOriginPoint(bounds.width()/2, bounds.height()/2);

	newpic->setPos(-bounds.width()/2.0, -bounds.height()/2.0);
	newpic->setRotation(pic->rotation());

	// Scale image to fit screen, taking the rotation in account
	QRectF truebounds = newpic->mapRectToScene(bounds);
	qreal scale = calcScale(truebounds.size(), view.size());
	newpic->setScale(scale);

	delete m_picture;
	m_picture = newpic;
	m_scene->addItem(m_picture);
}

