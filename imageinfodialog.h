#ifndef IMAGEINFODIALOG_H
#define IMAGEINFODIALOG_H

#include <QDialog>

namespace Ui {
    class ImageInfoDialog;
}

class Gallery;
class Picture;

class ImageInfoDialog : public QDialog
{
    Q_OBJECT

public:
	explicit ImageInfoDialog(const Gallery *gallery, const Picture& picture, QWidget *parent = 0);
    ~ImageInfoDialog();

private:
	Ui::ImageInfoDialog *m_ui;
};

#endif // IMAGEINFODIALOG_H
