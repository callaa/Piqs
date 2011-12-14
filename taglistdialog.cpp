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
#include <QSqlQueryModel>
#include <QSqlQuery>

#include "taglistdialog.h"
#include "ui_taglistdialog.h"

#include "database.h"

TagListDialog::TagListDialog(const Database *database, QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::TagListDialog),
	m_database(database)
{
	m_ui->setupUi(this);

	m_model = new QSqlQueryModel(this);

	m_model->setHeaderData(0, Qt::Horizontal, tr("Tag"));
	m_model->setHeaderData(1, Qt::Horizontal, tr("Occurences"));

	m_ui->tableView->setModel(m_model);
	m_ui->tableView->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
	m_ui->tableView->setSortingEnabled(true);
	m_ui->tableView->horizontalHeader()->setSortIndicator(1, Qt::DescendingOrder);

	m_ui->tableView->setFocus();

	connect(m_ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(tagClicked(QModelIndex)));
	connect(m_ui->filterbox, SIGNAL(textChanged(QString)), this, SLOT(filterChange(QString)));

	// We have to sort the results ourselves, because QSqlQueryModel doesn't implement sorting.
	connect(m_ui->tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(sortChange()));

	filterChange("");
}

TagListDialog::~TagListDialog()
{
	delete m_ui;
}

void TagListDialog::tagClicked(const QModelIndex& index)
{
	emit query(m_model->data(m_model->index(index.row(), 0)).toString());
}

void TagListDialog::filterChange(const QString &text)
{
	// Get sort column
	int sorts = m_ui->tableView->horizontalHeader()->sortIndicatorSection();
	QString sort;
	if(sorts==1)
		sort = "piccount";
	else
		sort = "tag";
	if(m_ui->tableView->horizontalHeader()->sortIndicatorOrder()==Qt::AscendingOrder)
		sort += " ASC";
	else
		sort += " DESC";

	// Perform query.
	// Note. This query can count a tag multiple times per picture because of tagsets.
	QSqlQuery q(m_database->get());
	if(text.isEmpty()) {
		q.exec("SELECT tag, count(picid) AS piccount FROM tag JOIN tagmap USING(tagid) GROUP BY tagid ORDER BY " + sort);
	} else {
		q.prepare("SELECT tag, count(picid) AS piccount FROM tag JOIN tagmap USING(tagid) WHERE tag LIKE ? GROUP BY tagid ORDER BY " + sort);
		q.bindValue(0, "%" + text + "%");
		q.exec();
	}
	m_model->setQuery(q);
}

void TagListDialog::sortChange()
{
	filterChange(m_ui->filterbox->text());
}
