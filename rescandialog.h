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
#ifndef RESCANDIALOG_H
#define RESCANDIALOG_H

#include <QDialog>

class Gallery;

namespace Ui {
    class RescanDialog;
}

class RescanDialog : public QDialog
{
    Q_OBJECT

public:
	explicit RescanDialog(const Gallery *gallery, QWidget *parent = 0);
    ~RescanDialog();

	void setQuickmode(bool quickmode) { m_quickmode = quickmode; }

public slots:
	void rescan();
	void filesDone();

signals:
	void rescanComplete();


private:
	Ui::RescanDialog *m_ui;
	const Gallery *m_gallery;
	bool m_quickmode;
};

#endif // RESCANDIALOG_H
