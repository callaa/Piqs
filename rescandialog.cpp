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
