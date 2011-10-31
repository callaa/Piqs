#include "slideshowoptions.h"
#include "ui_slideshowoptions.h"

#include "database.h"

SlideshowOptions::SlideshowOptions(const Database *database, QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::SlideshowOptions),
	m_database(database)
{
	m_ui->setupUi(this);

	bool ok;
	double duration = database->getSetting("slideshow.delay").toDouble(&ok);
	if(!ok)
		duration = 10.0;
	m_ui->slideduration->setValue(duration);

	connect(this, SIGNAL(accepted()), this, SLOT(saveChanges()));
}

SlideshowOptions::~SlideshowOptions()
{
	delete m_ui;
}

void SlideshowOptions::saveChanges()
{
	m_database->saveSetting("slideshow.delay", m_ui->slideduration->value());
}
