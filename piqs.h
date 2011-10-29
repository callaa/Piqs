#ifndef PIQS_H
#define PIQS_H

#include <QtGui/QMainWindow>

class QListView;
class QStackedWidget;

class BrowserWidget;
class ImageView;
class Gallery;
class Picture;
class QAction;

class Piqs : public QMainWindow
{
    Q_OBJECT
public:
    Piqs(QWidget *parent = 0);
    ~Piqs();

public slots:
	//! Switch to picture viewing mode and show the picture
	void showPicture(const Picture &picture);

	//! Move selection and show in picture viewer
	void showNextPicture();

	//! Move selection and show in picture viewer
	void showPreviousPicture();

	//! Switch back to browser mode
	void showBrowser();

	//! Rescan gallery directories for new files
	void rescan();

protected:
	void closeEvent(QCloseEvent *e);

private:
	void initActions();

	QAction *makeAction(const QString& title, const char *icon, const QKeySequence& shortcut);

	//! Stack of view widgets
	QStackedWidget *m_viewstack;

	//! The main thumbnail list view
	BrowserWidget *m_browser;

	//! Image viewer
	ImageView *m_viewer;

	//! The gallery
	Gallery *m_gallery;

	// Actions
	QAction *m_act_open;
	QAction *m_act_rescan;
	QAction *m_act_exit;
};

#endif // PIQS_H
