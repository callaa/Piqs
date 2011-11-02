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

private:
	Ui::TagListDialog *m_ui;
	QSqlQueryModel *m_model;
};

#endif // TAGLISTDIALOG_H
