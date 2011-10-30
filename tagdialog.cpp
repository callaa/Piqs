#include <QDebug>
#include <QSqlQuery>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QProgressDialog>

#include "tagdialog.h"
#include "ui_tagdialog.h"
#include "util.h"
#include "tagset.h"
#include "tagimplications.h"

#include "database.h"

TagDialog::TagDialog(const Database *database, QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::TagDialog),
	m_database(database),
	m_aliasdirty(false)
{
	m_ui->setupUi(this);

	// Get aliases
	QSqlQuery q("SELECT alias, tag FROM tagalias ORDER BY tag ASC", database->get());
	int row=0;
	while(q.next()) {
		m_ui->aliastable->insertRow(row);
		m_ui->aliastable->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
		m_ui->aliastable->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
		++row;
	}
	// One empty row for new aliases
	m_ui->aliastable->insertRow(row);

	connect(m_ui->aliastable, SIGNAL(cellChanged(int,int)), this, SLOT(aliasTableChanged(int, int)));

	// Get rules
	q.exec("SELECT rule, tags FROM tagrule ORDER by ruleorder ASC");
	row = 0;
	while(q.next()) {
		m_ui->ruletable->insertRow(row);
		m_ui->ruletable->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
		m_ui->ruletable->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
		++row;
	}
	// One empty row for new rules
	m_ui->ruletable->insertRow(row);
	connect(m_ui->ruletable, SIGNAL(cellChanged(int,int)), this, SLOT(ruleTableChanged(int, int)));

	connect(this, SIGNAL(accepted()), this, SLOT(saveChanges()));
}

TagDialog::~TagDialog()
{
	delete m_ui;
}

void TagDialog::aliasTableChanged(int row, int col)
{
	m_aliasdirty = true;
	QString val = m_ui->aliastable->item(row, col)->data(Qt::DisplayRole).toString();
	if(val=="") {
		if(row<m_ui->aliastable->rowCount()-1)
			m_ui->aliastable->removeRow(row);
	} else {
		QString normalized = Util::cleanTagName(val);
		if(col==1) {
			// Special rule for tag column: the value may not appear in any alias row
			for(int i=0;i<m_ui->aliastable->rowCount()-1;++i) {
				if(m_ui->aliastable->item(i, 0)->data(Qt::DisplayRole).toString() == normalized) {
					normalized = "";
					break;
				}
			}
		}
		m_ui->aliastable->item(row, col)->setData(Qt::DisplayRole, normalized);
		if(row==m_ui->aliastable->rowCount()-1)
			m_ui->aliastable->insertRow(m_ui->aliastable->rowCount());
	}
}

void TagDialog::ruleTableChanged(int row, int col)
{
	m_ruledirty = true;
	QString val = m_ui->ruletable->item(row, col)->data(Qt::DisplayRole).toString();
	if(val=="") {
		if(row<m_ui->ruletable->rowCount()-1)
			m_ui->ruletable->removeRow(row);
	} else {
		if(col==1) {
			// clean up tags
			QStringList tags = val.split(',');
			QMutableListIterator<QString> i(tags);
			while(i.hasNext()) {
				QString str = Util::cleanTagName(i.next());
				if(str.length()==0)
					i.remove();
				else
					i.setValue(str);
			}
			m_ui->ruletable->item(row, 1)->setData(Qt::DisplayRole, tags.join(", "));
		}
		if(row==m_ui->ruletable->rowCount()-1)
			m_ui->ruletable->insertRow(m_ui->ruletable->rowCount());
	}
}

void TagDialog::saveChanges()
{
	if(m_aliasdirty) {
		// Save changes to aliases

		m_database->get().transaction();
		QSqlQuery q(m_database->get());
		q.exec("DELETE FROM tagalias");

		q.prepare("INSERT INTO tagalias VALUES (?, ?)");
		for(int i=0;i<m_ui->aliastable->rowCount()-1;++i) {
			q.bindValue(0, m_ui->aliastable->item(i, 0)->data(Qt::DisplayRole));
			q.bindValue(1, m_ui->aliastable->item(i, 1)->data(Qt::DisplayRole));
			q.exec();
		}
		m_database->get().commit();
	}

	if(m_ruledirty) {
		// Save changes to rules

		m_database->get().transaction();
		QSqlQuery q(m_database->get());
		q.exec("DELETE FROM tagrule");

		q.prepare("INSERT INTO tagrule VALUES (?, ?, ?)");
		for(int i=0;i<m_ui->ruletable->rowCount()-1;++i) {
			q.bindValue(0, m_ui->ruletable->item(i, 0)->data(Qt::DisplayRole));
			q.bindValue(1, i);
			q.bindValue(2, m_ui->ruletable->item(i, 1)->data(Qt::DisplayRole));
			q.exec();
		}
		m_database->get().commit();
	}

	if(m_aliasdirty | m_ruledirty) {
		QMessageBox msgbox;
		msgbox.setText(tr("Rebuild tag index?"));
		msgbox.setInformativeText("The tag index needs to be rebuilt for the new rules to take effect.");
		msgbox.setWindowTitle(tr("Tag rules changed"));
		msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgbox.setButtonText(QMessageBox::Yes, tr("Rebuild"));
		msgbox.setButtonText(QMessageBox::No, tr("Not now"));
		if(msgbox.exec() == QMessageBox::Yes) {
			rebuildTagIndex();
		}
	}
}

void TagDialog::rebuildTagIndex()
{
	TagImplications tagrules = TagImplications::load(m_database);

	QSqlQuery q(m_database->get());
	q.setForwardOnly(true);

	// Get the number of pictures (for progress display)
	q.exec("SELECT COUNT(*) FROM picture WHERE tags!=\"\"");
	q.next();
	int filecount = q.value(0).toInt();

	// Show progress dialog
	QProgressDialog progress(tr("Rebuilding tag index..."), tr("Abort"), 0, filecount);
	progress.setMinimumDuration(100);

	// Destroy and re-create tables
	m_database->get().transaction();
	m_database->createTagIndexTables(true);

	// Reinsert tags to index
	q.exec("SELECT picid, tags FROM picture WHERE tags!=\"\"");
	int pos=0;
	while(q.next()) {
		TagIdSet tagset(TagSet::parse(q.value(1).toString()), m_database, q.value(0).toInt());
		tagrules.apply(tagset);
		tagset.save(m_database);

		progress.setValue(++pos);
		if(progress.wasCanceled()) {
			m_database->get().rollback();
			break;
		}
	}

	m_database->get().commit();
}
