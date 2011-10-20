#ifndef PIQS_H
#define PIQS_H

#include <QtGui/QMainWindow>

class QListView;
class QStackedWidget;

class Piqs : public QMainWindow
{
    Q_OBJECT

public:
    Piqs(QWidget *parent = 0);
    ~Piqs();
private:
	//! Stack of view widgets
	QStackedWidget *viewstack;

	//! The main thumbnail list view
	QListView *mainview;
};

#endif // PIQS_H
