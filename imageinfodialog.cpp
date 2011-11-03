#include <QFileInfo>
#include <QImageReader>
#include <QDateTime>

#include "imageinfodialog.h"
#include "ui_imageinfodialog.h"
#include "picture.h"

ImageInfoDialog::ImageInfoDialog(const Gallery *gallery, const Picture& picture, QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::ImageInfoDialog)
{
	m_ui->setupUi(this);

	QFileInfo fileinfo(picture.fullpath(gallery));
	m_ui->namelabel->setText(fileinfo.fileName());
	m_ui->pathlabel->setText(fileinfo.path());
	m_ui->sizelabel->setText(QString::number(fileinfo.size())); // TODO human friendly format
	m_ui->createdlabel->setText(fileinfo.created().toString(Qt::DefaultLocaleShortDate));
	m_ui->hashlabel->setText(picture.hash());

	QImageReader reader(fileinfo.absoluteFilePath());
	m_ui->typelabel->setText(QString::fromLatin1(reader.format()));
	QSize size = reader.size();
	m_ui->widthlabel->setText(QString::number(size.width()));
	m_ui->heightlabel->setText(QString::number(size.height()));
}

ImageInfoDialog::~ImageInfoDialog()
{
	delete m_ui;
}
