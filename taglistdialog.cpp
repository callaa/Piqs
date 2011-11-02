#include <QSqlQueryModel>
#include <QSqlQuery>

#include "taglistdialog.h"
#include "ui_taglistdialog.h"

#include "database.h"

TagListDialog::TagListDialog(const Database *database, QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::TagListDialog)
{
	m_ui->setupUi(this);

	m_model = new QSqlQueryModel(this);
	// Note. This query can count a tag multiple times per picture because of tagsets.
	m_model->setQuery(QSqlQuery(
						"SELECT tag, count(picid) as piccount FROM tag JOIN tagmap USING(tagid) GROUP BY tagid ORDER BY piccount DESC",
						database->get()));

	m_model->setHeaderData(0, Qt::Horizontal, tr("Tag"));
	m_model->setHeaderData(1, Qt::Horizontal, tr("Occurences"));

	m_ui->tableView->setModel(m_model);
	m_ui->tableView->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);

	connect(m_ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(tagClicked(QModelIndex)));
}

TagListDialog::~TagListDialog()
{
	delete m_ui;
}

void TagListDialog::tagClicked(const QModelIndex& index)
{
	emit query(m_model->data(m_model->index(index.row(), 0)).toString());
}
