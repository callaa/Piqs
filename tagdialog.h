#ifndef TAGDIALOG_H
#define TAGDIALOG_H

#include <QDialog>

namespace Ui {
    class TagDialog;
}

class Gallery;

class TagDialog : public QDialog
{
    Q_OBJECT

public:
	explicit TagDialog(Gallery *gallery, QWidget *parent = 0);
    ~TagDialog();

protected slots:
	void saveChanges();

private:
	void rebuildTagIndex();

	Ui::TagDialog *m_ui;
	Gallery *m_gallery;
};

#endif // TAGDIALOG_H
