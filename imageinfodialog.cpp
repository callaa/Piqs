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
