#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>

#include "imageview.h"
#include "ui_imageview.h"

#include "gallery.h"
#include "picture.h"

ImageView::ImageView(const Gallery *gallery, QWidget *parent) :
    QWidget(parent),
	m_ui(new Ui::ImageView),
	m_gallery(gallery)
{
	m_ui->setupUi(this);

	m_scene = new QGraphicsScene();
	m_ui->view->setScene(m_scene);

	// Set action button icons
	m_ui->fitbutton->setIcon(QIcon::fromTheme("zoom-fit-best"));
	m_ui->origsizebutton->setIcon(QIcon::fromTheme("zoom-original"));
	m_ui->zoominbutton->setIcon(QIcon::fromTheme("zoom-in"));
	m_ui->zoomoutbutton->setIcon(QIcon::fromTheme("zoom-out"));

	m_ui->prevbutton->setIcon(QIcon::fromTheme("go-previous"));
	m_ui->nextbutton->setIcon(QIcon::fromTheme("go-next"));

	connect(m_ui->fitbutton, SIGNAL(clicked(bool)), this, SLOT(zoomfit(bool)));
	connect(m_ui->origsizebutton, SIGNAL(clicked()), this, SLOT(zoomorig()));
	connect(m_ui->zoominbutton, SIGNAL(clicked()), this, SLOT(zoomin()));
	connect(m_ui->zoomoutbutton, SIGNAL(clicked()), this, SLOT(zoomout()));
	connect(m_ui->nextbutton, SIGNAL(clicked()), this, SIGNAL(requestNext()));
	connect(m_ui->prevbutton, SIGNAL(clicked()), this, SIGNAL(requestPrev()));
}

ImageView::~ImageView()
{
	delete m_ui;
}

bool ImageView::isAutofit() const
{
	return m_ui->fitbutton->isChecked();
}

void ImageView::setPicture(const Picture &picture)
{
	m_picture = picture;
	m_scene->clear();

	qDebug() << picture.fullpath(m_gallery);
	QGraphicsPixmapItem *item = m_scene->addPixmap(QPixmap(picture.fullpath(m_gallery)));
	m_scene->setSceneRect(item->boundingRect());
	if(isAutofit())
		scaleToFit();
}

void ImageView::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape) {
		emit exitView();
	} else
		QWidget::keyPressEvent(e);
}

void ImageView::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);
	if(isAutofit())
		scaleToFit();
}

void ImageView::zoomin()
{
	m_ui->fitbutton->setChecked(false);
	m_ui->view->scale(2.0, 2.0);
}

void ImageView::zoomout()
{
	m_ui->fitbutton->setChecked(false);
	m_ui->view->scale(0.5, 0.5);
}

void ImageView::zoomfit(bool checked)
{
	if(checked)
		scaleToFit();
	else
		m_ui->view->resetTransform();
}

void ImageView::setAutofit(bool fit)
{
	m_ui->fitbutton->setChecked(fit);
}


void ImageView::scaleToFit()
{
	if(m_ui->view->items().count()==0)
		return;

	QSizeF is = m_ui->view->items().at(0)->boundingRect().size();
	QSizeF vs = m_ui->view->size();

	qreal isa = is.height() / is.width();
	qreal vsa = vs.height() / vs.width();

	qreal scale;
	if(isa > vsa)
		scale = vs.height() / is.height();
	else
		scale = vs.width() / is.width();

	m_ui->view->setTransform(QTransform::fromScale(scale, scale));
}


void ImageView::zoomorig()
{
	m_ui->fitbutton->setChecked(false);
	m_ui->view->resetTransform();
}
