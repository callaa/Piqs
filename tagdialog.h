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
#ifndef TAGDIALOG_H
#define TAGDIALOG_H

#include <QDialog>

namespace Ui {
    class TagDialog;
}

class Gallery;

class TagDialog : public QDialog
{
    Q_OBJECT

public:
	explicit TagDialog(Gallery *gallery, QWidget *parent = 0);
    ~TagDialog();

protected slots:
	void saveChanges();

private:
	void rebuildTagIndex();

	Ui::TagDialog *m_ui;
	Gallery *m_gallery;
};

#endif // TAGDIALOG_H
