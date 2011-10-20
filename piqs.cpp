#include <QMenuBar>
#include <QListView>
#include <QStackedWidget>
#include <QListView>

#include "piqs.h"
#include "thumbnailmodel.h"
#include "gallery.h"

Piqs::Piqs(QWidget *parent)
    : QMainWindow(parent)
{
	// Create actions

	// Create menus
	QMenu *filemenu = menuBar()->addMenu(tr("&File"));

	// Create the image browser view
	mainview = new QListView(this);
	Gallery *gallery = new Gallery(QDir());
	gallery->rescan();
	ThumbnailModel *tbmodel = new ThumbnailModel(gallery);


	mainview->setModel(tbmodel);
	mainview->setViewMode(QListView::IconMode);
	mainview->setResizeMode(QListView::Adjust);
	mainview->setUniformItemSizes(true);
	mainview->setSpacing(6);

	// Create the (single) image viewer view

	// The view stack for switching between image browser and image viewer
	viewstack = new QStackedWidget(this);
	viewstack->addWidget(mainview);
	setCentralWidget(viewstack);
}

Piqs::~Piqs()
{

}
