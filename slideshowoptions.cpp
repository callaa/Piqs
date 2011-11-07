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

	QString scaletype = database->getSetting("slideshow.scale").toString();
	if(scaletype.isEmpty())
		scaletype = "fit";

	if(scaletype=="fit")
		m_ui->scalefit->setChecked(true);
	else
		m_ui->scalefill->setChecked(true);

	m_ui->scaleupscale->setChecked(database->getSetting("slideshow.upscale").toBool());

	connect(this, SIGNAL(accepted()), this, SLOT(saveChanges()));
}

SlideshowOptions::~SlideshowOptions()
{
	delete m_ui;
}

void SlideshowOptions::saveChanges()
{
	m_database->saveSetting("slideshow.delay", m_ui->slideduration->value());
	QString type;
	if(m_ui->scalefit->isChecked())
		type = "fit";
	else
		type = "fill";
	m_database->saveSetting("slideshow.scale", type);
	m_database->saveSetting("slideshow.upscale", m_ui->scaleupscale->isChecked());
}
