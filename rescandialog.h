#ifndef RESCANDIALOG_H
#define RESCANDIALOG_H

#include <QDialog>

class Gallery;

namespace Ui {
    class RescanDialog;
}

class RescanDialog : public QDialog
{
    Q_OBJECT

public:
	explicit RescanDialog(const Gallery *gallery, QWidget *parent = 0);
    ~RescanDialog();

public slots:
	void rescan();
	void filesProcessed(int count);
	void filesDone();

signals:
	void rescanComplete();


private:
	Ui::RescanDialog *m_ui;
	const Gallery *m_gallery;
};

#endif // RESCANDIALOG_H
