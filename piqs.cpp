#include <QMenuBar>
#include <QListView>
#include <QStackedWidget>
#include <QListView>
#include <QDebug>
#include <QTimer>

#include "piqs.h"
#include "thumbnailmodel.h"
#include "gallery.h"
#include "browserwidget.h"
#include "imageview.h"
#include "picture.h"

#include "rescandialog.h"

Piqs::Piqs(QWidget *parent)
    : QMainWindow(parent)
{
	// Create actions
	initActions();

	// Create menus
	QMenu *filemenu = menuBar()->addMenu(tr("&Gallery"));
	filemenu->addAction(m_act_open);
	filemenu->addAction(m_act_rescan);
	filemenu->addSeparator();
	filemenu->addAction(m_act_exit);

	// Create main gallery object
	m_gallery = new Gallery(QDir(), this);
	//gallery->rescan();

	// Create browser view
	m_browser = new BrowserWidget(m_gallery);

	// Create the (single) image viewer view
	m_viewer = new ImageView(m_gallery);

	// The view stack for switching between image browser and image viewer
	m_viewstack = new QStackedWidget(this);
	m_viewstack->addWidget(m_browser);
	m_viewstack->addWidget(m_viewer);
	setCentralWidget(m_viewstack);

	connect(m_browser, SIGNAL(pictureSelected(Picture)), this, SLOT(showPicture(Picture)));
	connect(m_viewer, SIGNAL(exitView()), this, SLOT(showBrowser()));
	connect(m_viewer, SIGNAL(requestNext()), this, SLOT(showNextPicture()));
	connect(m_viewer, SIGNAL(requestPrev()), this, SLOT(showPreviousPicture()));

	restoreGeometry(QByteArray::fromBase64(m_gallery->database()->getSetting("window.geometry").toByteArray()));
	m_viewer->setAutofit(m_gallery->database()->getSetting("viewer.autofit").toBool());

	if(m_gallery->totalCount()==0)
		rescan();
}

Piqs::~Piqs()
{

}

void Piqs::rescan()
{
	RescanDialog *rescan = new RescanDialog(m_gallery, this);
	connect(rescan, SIGNAL(rescanComplete()), m_browser, SLOT(refreshQuery()));
	QTimer::singleShot(0, rescan, SLOT(rescan()));
}

void Piqs::initActions()
{
	m_act_open = makeAction(tr("&Open"), "document-open", QKeySequence::Open);
	m_act_open->setDisabled(true); // TODO
	m_act_rescan = makeAction(tr("Rescan"), "edit-redo", QKeySequence());
	m_act_exit = makeAction(tr("E&xit"), "application-exit", QKeySequence::Quit);

	m_act_exit->setMenuRole(QAction::QuitRole);

	connect(m_act_rescan, SIGNAL(triggered()), this, SLOT(rescan()));
	connect(m_act_exit, SIGNAL(triggered()), this, SLOT(close()));
}

QAction *Piqs::makeAction(const QString& title, const char *icon, const QKeySequence& shortcut)
{
	QAction *act;
	QIcon qicon;
	if(icon)
			qicon = QIcon::fromTheme(icon);
	act = new QAction(qicon, title, this);
	if(shortcut.isEmpty()==false)
		act->setShortcut(shortcut);
	return act;
}

void Piqs::closeEvent(QCloseEvent *e)
{
	m_gallery->database()->saveSetting("window.geometry", saveGeometry().toBase64());
	m_gallery->database()->saveSetting("viewer.autofit", m_viewer->isAutofit());
	QMainWindow::closeEvent(e);
}

void Piqs::showPicture(const Picture &picture)
{
	qDebug() << "Show" << picture.id();
	m_viewstack->setCurrentIndex(1);
	m_viewer->setPicture(picture);
}

void Piqs::showBrowser()
{
	m_viewstack->setCurrentIndex(0);
}

void Piqs::showNextPicture()
{
	int sel = m_browser->getCurrentSelection() + 1;
	if(sel<m_browser->getPictureCount()) {
		m_browser->selectPicture(sel);
		showPicture(*m_browser->getPictureAt(sel));
	}
}

void Piqs::showPreviousPicture()
{
	int sel = m_browser->getCurrentSelection() - 1;
	if(sel>=0) {
		m_browser->selectPicture(sel);
		showPicture(*m_browser->getPictureAt(sel));
	}
}
