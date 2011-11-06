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
#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenuBar>
#include <QListView>
#include <QStackedWidget>
#include <QListView>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>
#include <QDesktopWidget>

#include <cstdlib> // for RAND_MAX

#include "piqs.h"
#include "thumbnailmodel.h"
#include "gallery.h"
#include "browserwidget.h"
#include "imageview.h"
#include "picture.h"
#include "tagdialog.h"
#include "taglistdialog.h"
#include "slideshowoptions.h"

#include "rescandialog.h"
#include "slideshow.h"

Piqs::Piqs(const QString& root, QWidget *parent)
    : QMainWindow(parent)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

	// Create actions
	initActions();

	// Create menus
	QMenu *filemenu = menuBar()->addMenu(tr("&Gallery"));
	filemenu->addAction(m_act_open);
	filemenu->addSeparator();
	filemenu->addAction(m_act_rescan);
	filemenu->addAction(m_act_tagrules);
	filemenu->addAction(m_act_taglist);
	filemenu->addSeparator();
	filemenu->addAction(m_act_exit);

	QMenu *querymenu = menuBar()->addMenu(tr("&Query"));
	querymenu->addAction(makeQueryAction(tr("&All"), ":all"));
	querymenu->addAction(makeQueryAction(tr("&Untagged"), ":untagged"));
	querymenu->addAction(makeQueryAction(tr("&New"), ":new"));
	querymenu->addAction(makeQueryAction(tr("&Hidden"), ":hidden"));
	querymenu->addAction(makeQueryAction(tr("&Missing"), ":missing"));
	querymenu->addAction(makeQueryAction(tr("&Duplicates"), ":duplicate"));
	querymenu->addAction(makeQueryAction(tr("&File..."), ":file(%1)", tr("Filename")));
	querymenu->addAction(makeQueryAction(tr("&Title..."), ":title(%1)", tr("Title")));
	querymenu->addAction(makeQueryAction(tr("Ha&sh..."), ":hash(%1)", tr("SHA-1 hash")));

	connect(querymenu, SIGNAL(triggered(QAction*)), this, SLOT(queryMenuTriggered(QAction*)));

	QMenu *slidemenu = menuBar()->addMenu(tr("&Slideshow"));
	slidemenu->addAction(m_act_slideshow);
	slidemenu->addSeparator();
	slidemenu->addAction(m_act_slideselected);
	slidemenu->addAction(m_act_slideshuffle);
	slidemenu->addSeparator();
	slidemenu->addAction(m_act_slideshowopts);

	QMenu *helpmenu = menuBar()->addMenu(tr("&Help"));
	QAction *about = helpmenu->addAction(tr("About..."));
	QAction *aboutqt = helpmenu->addAction(tr("About Qt..."));

	connect(about, SIGNAL(triggered()), this, SLOT(showAboutBox()));
	connect(aboutqt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));

	// Create main gallery object
	m_gallery = new Gallery(QDir(root), this);

	if(!m_gallery->isOk()) {
		QMessageBox::critical(0, tr("Error"), tr("Couldn't open gallery"));
		this->deleteLater();
		return;
	} else {
		// Set window title
		this->setWindowTitle(QString("%1 - Piqs").arg(QDir(m_gallery->root().absolutePath()).dirName()));
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
		connect(m_viewer, SIGNAL(changed()), m_browser, SLOT(refreshQuery()));

		// Set default window size
		QByteArray oldgeometry = m_gallery->database()->getSetting("window.geometry").toByteArray();
		if(oldgeometry.isEmpty()) {
				QRect screen = QApplication::desktop()->screenGeometry();
				resize(screen.width() * 2 / 3, screen.height() * 2 / 3);
		} else {
			restoreGeometry(QByteArray::fromBase64(oldgeometry));
		}

		m_viewer->setAutofit(m_gallery->database()->getSetting("viewer.autofit").toBool());

		if(m_gallery->totalCount()==0)
			rescan();
	}
}

Piqs::~Piqs()
{

}

void Piqs::showAboutBox()
{
	QMessageBox::about(this, "Piqs",
					   "Piqs 0.1.0\n"
					   "A tagging image browser\n\n"
					   "Copyright 2011 Calle Laakkonen <calle@luolamies.org>\n"
					   "Piqs is distributed under the GNU General Public License Version 3"
					   );
}

void Piqs::showOpenDialog()
{
	QString dir = QFileDialog::getExistingDirectory(this);
	if(dir.isEmpty())
		return;

	// Check that the gallery hasn't been opened already
	foreach(QWidget *toplevel, QApplication::topLevelWidgets()) {
		Piqs *piqs = qobject_cast<Piqs*>(toplevel);
		if(piqs!=0 && piqs->isGalleryDir(dir)) {
			qDebug() << "found existing window for" << dir;
			piqs->raise();
			piqs->activateWindow();
			return;
		}
	}

	// Gallery doesn't seem to be open already.
	qDebug() << "opening new window for" << dir;
	(new Piqs(dir))->show();
}

bool Piqs::isGalleryDir(const QString& dir) const
{
	return dir.startsWith(m_gallery->root().absolutePath());
}

void Piqs::rescan()
{
	RescanDialog *rescan = new RescanDialog(m_gallery, this);
	connect(rescan, SIGNAL(rescanComplete()), m_browser, SLOT(refreshQuery()));
	QTimer::singleShot(0, rescan, SLOT(rescan()));
}

void Piqs::showTagrules()
{
	TagDialog *dialog = new TagDialog(m_gallery, this);
	dialog->setAttribute(Qt::WA_DeleteOnClose, true);
	dialog->setModal(true);
	dialog->show();
}

void Piqs::showTaglist()
{
	TagListDialog *dialog = new TagListDialog(m_gallery->database(), this);
	connect(dialog, SIGNAL(query(QString)), m_browser, SLOT(setQuery(QString)));
	connect(dialog, SIGNAL(query(QString)), this, SLOT(showBrowser()));
	dialog->setAttribute(Qt::WA_DeleteOnClose, true);
	dialog->setModal(true);
	dialog->show();
}

void Piqs::initActions()
{
	m_act_open = makeAction(tr("&Open..."), "document-open", QKeySequence::Open);
	m_act_rescan = makeAction(tr("Rescan"), "edit-redo");
	m_act_tagrules = makeAction(tr("&Tag rules..."), "configure");
	m_act_taglist = makeAction(tr("Tag list..."), 0);
	m_act_exit = makeAction(tr("E&xit"), "application-exit", QKeySequence::Quit);

	m_act_exit->setMenuRole(QAction::QuitRole);

	connect(m_act_open, SIGNAL(triggered()), this, SLOT(showOpenDialog()));
	connect(m_act_rescan, SIGNAL(triggered()), this, SLOT(rescan()));
	connect(m_act_exit, SIGNAL(triggered()), this, SLOT(close()));
	connect(m_act_tagrules, SIGNAL(triggered()), this, SLOT(showTagrules()));
	connect(m_act_taglist, SIGNAL(triggered()), this, SLOT(showTaglist()));

	m_act_slideshow = makeAction(tr("&Start"), "media-playback-start", QKeySequence("F9"));
	m_act_slideselected = makeAction(tr("Limit to selection"), 0);
	m_act_slideselected->setCheckable(true);
	m_act_slideselected->setChecked(true);
	m_act_slideshuffle= makeAction(tr("Shuffle"), 0);
	m_act_slideshuffle->setCheckable(true);
	m_act_slideshowopts = makeAction(tr("Options..."), "configure");

	connect(m_act_slideshow, SIGNAL(triggered()), this, SLOT(startSlideshow()));
	connect(m_act_slideshowopts, SIGNAL(triggered()), this, SLOT(showSlideshowOptions()));
}

QAction *Piqs::makeAction(const QString& title, const char *icon, const QKeySequence& shortcut)
{
	QIcon qicon;
	if(icon)
			qicon = QIcon::fromTheme(icon);
	QAction *act = new QAction(qicon, title, this);
	if(shortcut.isEmpty()==false)
		act->setShortcut(shortcut);
	return act;
}

QAction *Piqs::makeQueryAction(const QString& title, const QString& query, const QString& prompt)
{
	QAction *act = new QAction(title, this);
	act->setProperty("querystring", query);
	act->setProperty("queryprompt", prompt);
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
	m_viewstack->setCurrentIndex(1);
	m_viewer->setPicture(picture);
}

void Piqs::showBrowser()
{
	m_viewstack->setCurrentIndex(0);
}

void Piqs::queryMenuTriggered(QAction *action)
{
	QString q = action->property("querystring").toString();
	if(q.isEmpty())
		return;

	// Special case
	if(q==":all")
		q = "";

	QString prompt = action->property("queryprompt").toString();
	if(!prompt.isEmpty()) {
		QString val = QInputDialog::getText(this, tr("Query"), prompt);
		if(val.isEmpty())
			return;
		q = q.arg(val);
	}

	showBrowser();
	m_browser->setQuery(q);
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

void Piqs::startSlideshow()
{
	const int totalcount = m_browser->getThumbnailModel()->rowCount(QModelIndex());
	if(totalcount==0)
		return;

	QVector<int> selection;
	if(m_act_slideselected->isChecked())
		selection = m_browser->getSelection();

	if(m_act_slideshuffle->isChecked()) {
		// If selection is too short, select all
		if(selection.count() <= 1) {
			selection = QVector<int>(totalcount);
			for(int i=0;i<totalcount;++i)
				selection[i] = i;
		}

		// Shuffle selection vector
		for(int i=selection.count()-1;i>0;--i) {
			int j = qrand() / ( RAND_MAX / i + 1 );
			qSwap(selection[i], selection[j]);
		}
	}
	Slideshow *slideshow = new Slideshow(m_gallery, m_browser->getThumbnailModel(), selection, this);
	slideshow->start();
}

void Piqs::showSlideshowOptions()
{
	SlideshowOptions *dialog = new SlideshowOptions(m_gallery->database(), this);
	dialog->setAttribute(Qt::WA_DeleteOnClose, true);
	dialog->setModal(true);
	dialog->show();
}
