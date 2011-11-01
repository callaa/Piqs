#include <QDebug>
#include <QSqlQuery>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QProgressDialog>
#include <QVariant>
#include <QBuffer>

#include "tagdialog.h"
#include "ui_tagdialog.h"
#include "util.h"
#include "tagset.h"
#include "tagrules.h"
#include "tags.h"

#include "gallery.h"
#include "database.h"

TagDialog::TagDialog(Gallery *gallery, QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::TagDialog),
	m_gallery(gallery)
{
	m_ui->setupUi(this);

	// Get tag rules
	m_ui->rulesedit->setPlainText(m_gallery->database()->getSetting("tagrules").toString());

	connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveChanges()));
}

TagDialog::~TagDialog()
{
	delete m_ui;
}

void TagDialog::saveChanges()
{
	QString rulestring = m_ui->rulesedit->toPlainText();
	QList<TagRuleExpression> rules;

	try {
		QByteArray rulechrs(rulestring.toLocal8Bit());
		QBuffer rulebuffer(&rulechrs);
		rulebuffer.open(QIODevice::ReadOnly|QIODevice::Text);

		rules = TagImplications::parseRuleFile("", rulebuffer, m_gallery->metadir());
	} catch(const TagRuleParseException& e) {
		QString msg;
		if(e.file.length()>0)
			msg = e.file + ":";
		msg = msg + ":" + QString::number(e.line) + ":" + e.message;

		QMessageBox warning(QMessageBox::Warning, tr("Parse error"), msg);
		warning.setStandardButtons(QMessageBox::Ignore | QMessageBox::Cancel);
		if(warning.exec()==QMessageBox::Ignore) {
			// User can ignore the warning and save the invalid rules anyway.
			// In this case, the tag index is not rebuilt and the active rules are not changed
			m_gallery->database()->saveSetting("tagrules", rulestring);
			accept();
		}
		return;
	}

	// Save original rule source code
	m_gallery->database()->saveSetting("tagrules", rulestring);

	// Save rules for use
	m_gallery->database()->get().transaction();
	{
		QSqlQuery q(m_gallery->database()->get());
		q.exec("DELETE FROM tagalias");
		q.exec("DELETE FROM tagrule");

		int rulecount=0;
		foreach(const TagRuleExpression &expr, rules) {
			if(expr.type == TagRuleExpression::ALIAS) {
				q.prepare("INSERT INTO tagalias VALUES (?, ?)");
				q.bindValue(0, expr.rule);
				q.bindValue(1, expr.tagset);
				q.exec();
			} else {
				q.prepare("INSERT INTO tagrule VALUES (?, ?, ?)");
				q.bindValue(0, expr.rule);
				q.bindValue(1, rulecount++);
				q.bindValue(2, expr.tagset);
				q.exec();
			}
		}
	}
	m_gallery->database()->get().commit();

	// Rebuild tag index?
	QMessageBox msgbox(QMessageBox::Question, tr("Tag rules changed"), tr("Rebuild tag index?"));
	msgbox.setInformativeText(tr("This will apply the new rules to all existing tags."));
	msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgbox.setButtonText(QMessageBox::Yes, tr("Rebuild"));
	msgbox.setButtonText(QMessageBox::No, tr("Not now"));
	if(msgbox.exec() == QMessageBox::Yes) {
		rebuildTagIndex();
	}

	accept();
}

void TagDialog::rebuildTagIndex()
{
	// Get the number of pictures. This is needed for the progress bar
	// and sqlite doesn't return the size of the result set.
	int filecount;
	{
		QSqlQuery q("SELECT COUNT(*) FROM picture WHERE tags!=\"\"", m_gallery->database()->get());
		if(!q.next()) {
			Database::showError("Couldn't get file count!", q);
			return;
		}
		filecount = q.value(0).toInt();
	}

	// Show progress dialog
	QProgressDialog progress(tr("Rebuilding tag index..."), tr("Abort"), 0, filecount);
	progress.setMinimumDuration(100);

	// Destroy and re-create tag index tables
	m_gallery->database()->get().transaction();
	m_gallery->database()->tags()->createTables(true);

	TagImplications tagrules = TagImplications::load(m_gallery->database());

	// Reinsert tags to index
	QSqlQuery q(m_gallery->database()->get());
	q.setForwardOnly(true);
	q.exec("SELECT picid, tags FROM picture WHERE tags!=\"\"");
	int pos=0;
	while(q.next()) {
		TagIdSet tagset(TagSet::parse(q.value(1).toString()), m_gallery->database()->tags(), q.value(0).toInt());
		tagrules.apply(tagset);
		tagset.save(m_gallery->database(), false);

		progress.setValue(++pos);
		if(progress.wasCanceled()) {
			m_gallery->database()->get().rollback();
			break;
		}
	}

	m_gallery->database()->get().commit();
}
