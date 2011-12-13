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
#ifndef PIQS_H
#define PIQS_H

#include <QMainWindow>

class QListView;
class QStackedWidget;
class QLabel;

class BrowserWidget;
class ImageView;
class Gallery;
class Picture;
class QAction;

class Piqs : public QMainWindow
{
    Q_OBJECT
public:
	Piqs(const QString& root, QWidget *parent = 0);
    ~Piqs();

	//! Check if the given path is this gallery's root or subdirectory
	bool isGalleryDir(const QString& dir) const;

public slots:
	//! Switch to picture viewing mode and show the picture
	void showPicture(const Picture &picture);

	//! Move selection and show in picture viewer
	void showNextPicture();

	//! Move selection and show in picture viewer
	void showPreviousPicture();

	//! Switch back to browser mode
	void showBrowser();

	//! Start a slideshow
	void startSlideshow();

	//! Set the status bar label showing the number of pictures
	void setPictureCount(int shown, int total);

protected slots:
	void queryMenuTriggered(QAction *action);
	void showAboutBox();

	//! Show the tag rules dialog
	void showTagrules();

	//! Show the tag list dialog
	void showTaglist();

	//! Rescan gallery directories new and changed files
	void rescan();

	//! Rescan gallery directories for new files
	void quickscan();

	//! Show slideshow options dialog
	void showSlideshowOptions();

	//! Show dialog for opening a new main window instance
	void showOpenDialog();

protected:
	void closeEvent(QCloseEvent *e);

private:
	void initActions();

	QAction *makeAction(const QString& title, const char *icon, const QString& statustip, const QKeySequence& shortcut=QKeySequence());

	QAction *makeQueryAction(const QString& title, const QString& query, const QString& statustip, const QString& prompt=QString());

	//! Stack of view widgets
	QStackedWidget *m_viewstack;

	//! The main thumbnail list view
	BrowserWidget *m_browser;

	//! Image viewer
	ImageView *m_viewer;

	//! The gallery
	Gallery *m_gallery;

	//! Picture count status bar label
	QLabel *m_piccount;

	// Actions
	QAction *m_act_open;
	QAction *m_act_rescan;
	QAction *m_act_quickscan;
	QAction *m_act_tagrules;
	QAction *m_act_taglist;
	QAction *m_act_exit;

	QAction *m_act_slideshow;
	QAction *m_act_slideselected;
	QAction *m_act_slideshuffle;
	QAction *m_act_slideshowopts;
};

#endif // PIQS_H
