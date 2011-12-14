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
#ifndef TAGLISTDIALOG_H
#define TAGLISTDIALOG_H

#include <QDialog>

namespace Ui {
    class TagListDialog;
}

class QSqlQueryModel;
class QModelIndex;
class Database;

class TagListDialog : public QDialog
{
    Q_OBJECT

public:
	explicit TagListDialog(const Database *database, QWidget *parent = 0);
    ~TagListDialog();

signals:
	//! User selected a tag for querying
	void query(const QString& query);

private slots:
	void tagClicked(const QModelIndex& index);
	void filterChange(const QString& text);
	void sortChange();

private:
	Ui::TagListDialog *m_ui;
	QSqlQueryModel *m_model;
	const Database *m_database;
};

#endif // TAGLISTDIALOG_H
