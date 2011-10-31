#ifndef SLIDESHOWOPTIONS_H
#define SLIDESHOWOPTIONS_H

#include <QDialog>

class Database;

namespace Ui {
    class SlideshowOptions;
}

class SlideshowOptions : public QDialog
{
    Q_OBJECT

public:
	explicit SlideshowOptions(const Database *database, QWidget *parent = 0);
    ~SlideshowOptions();

protected slots:
	void saveChanges();

private:
	Ui::SlideshowOptions *m_ui;
	const Database *m_database;
};

#endif // SLIDESHOWOPTIONS_H
