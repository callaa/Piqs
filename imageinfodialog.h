#ifndef IMAGEINFODIALOG_H
#define IMAGEINFODIALOG_H

#include <QDialog>

namespace Ui {
    class ImageInfoDialog;
}

class ImageInfoDialog : public QDialog
{
    Q_OBJECT

public:
	explicit ImageInfoDialog(const QString& file, QWidget *parent = 0);
    ~ImageInfoDialog();

private:
	Ui::ImageInfoDialog *m_ui;
};

#endif // IMAGEINFODIALOG_H
