#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QMenu>

#include "imageview.h"
#include "ui_imageview.h"

#include "gallery.h"
#include "picture.h"
#include "tagset.h"
#include "tagvalidator.h"
#include "tagcompleter.h"
#include "imageinfodialog.h"

ImageView::ImageView(Gallery *gallery, QWidget *parent) :
    QWidget(parent),
	m_ui(new Ui::ImageView),
	m_gallery(gallery)
{
	m_ui->setupUi(this);

	m_scene = new QGraphicsScene();
	m_ui->view->setScene(m_scene);

	m_ui->tagedit->setValidator(new TagValidator());
	//m_ui->tagedit->setCompleter(new TagCompleter(gallery->database()->tags()));

	// Create actions
	QAction *next = new QAction(QIcon::fromTheme("go-next"), tr("Next"), this);
	next->setShortcut(QKeySequence("Ctrl+."));

	QAction *prev = new QAction(QIcon::fromTheme("go-previous"), tr("Previous"), this);
	prev->setShortcut(QKeySequence("Ctrl+,"));

	QAction *zoomin = new QAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), this);
	zoomin->setShortcut(QKeySequence("Ctrl++"));

	QAction *zoomout = new QAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), this);
	zoomout->setShortcut(QKeySequence("Ctrl+-"));

	QAction *zoomorig = new QAction(QIcon::fromTheme("zoom-original"), tr("Original size"), this);
	zoomorig->setShortcut(QKeySequence("Ctrl+0"));

	m_autofit = new QAction(QIcon::fromTheme("zoom-fit-best"), tr("Fit to screen"), this);
	m_autofit->setShortcut(QKeySequence("Ctrl+1"));
	m_autofit->setCheckable(true);
	
	QAction *imginfo = new QAction(tr("Information..."), this);

	// Connect actions
	connect(next, SIGNAL(triggered()), this, SIGNAL(requestNext()));
	connect(prev, SIGNAL(triggered()), this, SIGNAL(requestPrev()));

	connect(zoomin, SIGNAL(triggered()), this, SLOT(zoomin()));
	connect(zoomout, SIGNAL(triggered()), this, SLOT(zoomout()));
	connect(m_autofit, SIGNAL(triggered(bool)), this, SLOT(zoomfit(bool)));
	connect(zoomorig, SIGNAL(triggered()), this, SLOT(zoomorig()));
	connect(imginfo, SIGNAL(triggered()), this, SLOT(showInfo()));

	// Create image context menu
	m_imgctxmenu = new QMenu(this);
	m_imgctxmenu->addAction(zoomin);
	m_imgctxmenu->addAction(zoomout);
	m_imgctxmenu->addAction(zoomorig);
	m_imgctxmenu->addAction(m_autofit);
	m_imgctxmenu->addSeparator();
	m_imgctxmenu->addAction(next);
	m_imgctxmenu->addAction(prev);
	m_imgctxmenu->addAction(imginfo);

	m_ui->view->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_ui->view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(pictureContextMenu(QPoint)));

	// Connect actions to buttons
	m_ui->nextbutton->setDefaultAction(next);
	m_ui->prevbutton->setDefaultAction(prev);
	m_ui->zoominbutton->setDefaultAction(zoomin);
	m_ui->zoomoutbutton->setDefaultAction(zoomout);
	m_ui->fitbutton->setDefaultAction(m_autofit);
	m_ui->origsizebutton->setDefaultAction(zoomorig);

	// Connect input edit events
	connect(m_ui->titleedit, SIGNAL(returnPressed()), this, SLOT(saveTitle()));
	connect(m_ui->tagedit, SIGNAL(returnPressed()), this, SLOT(saveTags()));
}

ImageView::~ImageView()
{
	delete m_ui;
}

void ImageView::pictureContextMenu(const QPoint& point)
{
	m_imgctxmenu->popup(m_ui->view->mapToGlobal(point));
}

void ImageView::showInfo()
{
	ImageInfoDialog *info = new ImageInfoDialog(m_gallery, m_picture);
	info->setAttribute(Qt::WA_DeleteOnClose, true);
	info->show();
}

bool ImageView::isAutofit() const
{
	return m_autofit->isChecked();
}

void ImageView::setPicture(const Picture &picture)
{
	m_picture = picture;
	m_scene->clear();

	QGraphicsPixmapItem *item = m_scene->addPixmap(QPixmap(picture.fullpath(m_gallery)));
	m_scene->setSceneRect(item->boundingRect());
	if(isAutofit())
		scaleToFit();

	m_ui->titleedit->setText(picture.title());
	m_ui->tagedit->setText(picture.tagString());
	m_ui->tagedit->setFocus();
	m_ui->alltags->setText(TagSet::getForPicture(m_gallery->database(), picture.id()).toString());
}

void ImageView::saveTitle()
{
	m_picture.saveTitle(m_gallery->database(), m_ui->titleedit->text());
	emit changed();
}

void ImageView::saveTags()
{
	m_picture.saveTags(m_gallery->database(), m_ui->tagedit->text());
	m_ui->alltags->setText(TagSet::getForPicture(m_gallery->database(), m_picture.id()).toString());
	emit changed();
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
	m_autofit->setChecked(false);
	m_ui->view->scale(2.0, 2.0);
}

void ImageView::zoomout()
{
	m_autofit->setChecked(false);
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
	m_autofit->setChecked(fit);
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

	// Only downscale
	if(scale > 1.0)
		scale = 1.0;

	m_ui->view->setTransform(QTransform::fromScale(scale, scale));
}


void ImageView::zoomorig()
{
	m_autofit->setChecked(false);
	m_ui->view->resetTransform();
}
