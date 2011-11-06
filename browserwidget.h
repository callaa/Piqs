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
#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H

#include <QWidget>

class QListView;
class QLineEdit;
class QModelIndex;
class QMenu;

class Gallery;
class Picture;
class ThumbnailModel;

//! Image thumbnail browser widget
class BrowserWidget : public QWidget
{
    Q_OBJECT
public:
	explicit BrowserWidget(Gallery *gallery, QWidget *parent = 0);

	//! Get the picture at the specific index
	const Picture *getPictureAt(int index) const;

	//! Get the number of pictures in this view
	int getPictureCount() const;

	//! Get the index of the currently selected image
	int getCurrentSelection() const;

	//! Get all currently selected indexes
	QVector<int> getSelection() const;

	//! Set the selection
	void selectPicture(int index);

	ThumbnailModel *getThumbnailModel() { return m_model; }

signals:
	//! User selected a picture for closer viewing
	void pictureSelected(const Picture& picture);

public slots:
	//! Make a new query
	void setQuery(const QString& query);

	//! Emit pictureSelected for the picture at index
	void openPicture(const QModelIndex& index);

	//! Invalidate cache and refresh view
	void refreshQuery();

	//! Perform a new query using the search string in the query box
	void updateQuery();

	//! Uncache the currently selected picture (e.g. because metadata has been changed)
	void uncacheSelected();

protected slots:
	void pictureContextMenu(const QPoint& point);

	void picSelectedAddtags();
	void picSelectedInfo();
	void picSelectedShow();
	void picSelectedHide();
	void picSelectedDelete();
	void picSelectedSetHidden(bool hidden);

private:
	Gallery *m_gallery;
	QListView *m_view;
	QLineEdit *m_searchbox;
	ThumbnailModel *m_model;
	QMenu *m_viewctxmenu;
};

#endif // BROWSERWIDGET_H
