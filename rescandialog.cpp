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
	connect(thread, SIGNAL(filesAdded(int)), this, SLOT(filesProcessed(int)));
	connect(thread, SIGNAL(finished()), this, SLOT(filesDone()));
	connect(thread, SIGNAL(finished()), this, SIGNAL(rescanComplete()));
	connect(this, SIGNAL(rejected()), thread, SLOT(abortScan()));

	show();
	thread->start();
}

void RescanDialog::filesProcessed(int count)
{
	m_ui->picturecount->display(count);
}

void RescanDialog::filesDone()
{
	m_ui->statuslabel->setText(tr("Done."));
	m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	m_ui->buttonBox->button(QDialogButtonBox::Abort)->setEnabled(false);
}
