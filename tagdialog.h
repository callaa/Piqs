#ifndef TAGDIALOG_H
#define TAGDIALOG_H

#include <QDialog>

class QModelIndex;

namespace Ui {
    class TagDialog;
}

class Database;

class TagDialog : public QDialog
{
    Q_OBJECT

public:
	explicit TagDialog(const Database *database, QWidget *parent = 0);
    ~TagDialog();

protected slots:
	void aliasTableChanged(int row, int col);
	void ruleTableChanged(int row, int col);

	void saveChanges();

private:
	void rebuildTagIndex();

	Ui::TagDialog *m_ui;
	const Database *m_database;
	bool m_aliasdirty, m_ruledirty;
};

#endif // TAGDIALOG_H
