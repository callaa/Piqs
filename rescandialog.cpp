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
#include <QDebug>
#include <QPushButton>

#include "rescandialog.h"
#include "rescanthread.h"

#include "ui_rescandialog.h"

RescanDialog::RescanDialog(const Gallery *gallery, QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::RescanDialog),
	m_gallery(gallery)
{
	m_ui->setupUi(this);
	m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	setModal(true);
	setAttribute(Qt::WA_DeleteOnClose, true);

}

RescanDialog::~RescanDialog()
{
	qDebug() << "rescan dialog destructed";
	delete m_ui;
}

void RescanDialog::rescan()
{
	RescanThread *thread = new RescanThread(m_gallery, this);
	connect(thread, SIGNAL(statusChanged(QString)), m_ui->statuslabel, SLOT(setText(QString)));
	connect(thread, SIGNAL(filesAdded(int)), m_ui->picturecount, SLOT(display(int)));
	connect(thread, SIGNAL(foldersSearched(int)), m_ui->foldercount, SLOT(display(int)));
	connect(thread, SIGNAL(missingFound(int)), m_ui->missingcount, SLOT(display(int)));
	connect(thread, SIGNAL(dupesFound(int)), m_ui->dupecount, SLOT(display(int)));
	connect(thread, SIGNAL(movesFound(int)), m_ui->movecount, SLOT(display(int)));

	connect(thread, SIGNAL(finished()), this, SLOT(filesDone()));
	connect(thread, SIGNAL(finished()), this, SIGNAL(rescanComplete()));
	connect(this, SIGNAL(rejected()), thread, SLOT(abortScan()));

	show();
	thread->start();
}

void RescanDialog::filesDone()
{
	m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	m_ui->buttonBox->button(QDialogButtonBox::Abort)->setEnabled(false);
}
