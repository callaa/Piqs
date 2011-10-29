#include <QListView>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "browserwidget.h"
#include "gallery.h"
#include "thumbnailmodel.h"

BrowserWidget::BrowserWidget(Gallery *gallery, QWidget *parent) :
	QWidget(parent), m_gallery(gallery)
{
	QVBoxLayout *mainlayout = new QVBoxLayout(this);
	setLayout(mainlayout);

	m_model = new ThumbnailModel(gallery);
	m_model->setQuery("");

	m_view = new QListView();
	m_view->setModel(m_model);
	m_view->setViewMode(QListView::IconMode);
	m_view->setResizeMode(QListView::Adjust);
	m_view->setUniformItemSizes(true);
	m_view->setSpacing(6);
	mainlayout->addWidget(m_view);

	QHBoxLayout *searchlayout = new QHBoxLayout();
	mainlayout->addLayout(searchlayout);

	QLabel *searchlbl = new QLabel(tr("Search:"));
	searchlayout->addWidget(searchlbl);

	m_searchbox = new QLineEdit();
	searchlayout->addWidget(m_searchbox);

	connect(m_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openPicture(QModelIndex)));
}

void BrowserWidget::openPicture(const QModelIndex& index)
{
	const Picture *pic = m_model->pictureAt(index.row());
	if(pic!=0)
		emit pictureSelected(*pic);
}

void BrowserWidget::refreshQuery()
{
	m_model->refreshQuery();
}

const Picture *BrowserWidget::getPictureAt(int index) const
{
	return m_model->pictureAt(index);
}

int BrowserWidget::getPictureCount() const
{
	return m_model->rowCount(QModelIndex());
}

int BrowserWidget::getCurrentSelection() const
{
	return m_view->selectionModel()->currentIndex().row();
}

void BrowserWidget::selectPicture(int index)
{
	m_view->selectionModel()->setCurrentIndex(m_model->index(index), QItemSelectionModel::SelectCurrent);
}
