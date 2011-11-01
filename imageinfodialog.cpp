#include <QFileInfo>
#include <QImageReader>

#include "imageinfodialog.h"
#include "ui_imageinfodialog.h"

ImageInfoDialog::ImageInfoDialog(const QString& file, QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::ImageInfoDialog)
{
	m_ui->setupUi(this);

	QFileInfo fileinfo(file);
	m_ui->namelabel->setText(fileinfo.fileName());
	m_ui->pathlabel->setText(fileinfo.path());
	m_ui->sizelabel->setText(QString::number(fileinfo.size())); // TODO human friendly format

	QImageReader reader(file);
	m_ui->typelabel->setText(QString::fromLatin1(reader.format()));
	QSize size = reader.size();
	m_ui->widthlabel->setText(QString::number(size.width()));
	m_ui->heightlabel->setText(QString::number(size.height()));

}

ImageInfoDialog::~ImageInfoDialog()
{
	delete m_ui;
}
