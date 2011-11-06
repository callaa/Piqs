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
