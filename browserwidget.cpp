#include <QListView>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>

#include "browserwidget.h"
#include "gallery.h"
#include "thumbnailmodel.h"
#include "tagquery.h"
#include "tagcompleter.h"

BrowserWidget::BrowserWidget(Gallery *gallery, QWidget *parent) :
	QWidget(parent), m_gallery(gallery)
{
	QVBoxLayout *mainlayout = new QVBoxLayout(this);
	setLayout(mainlayout);

	m_model = new ThumbnailModel(gallery);
	m_model->setQuery(ThumbnailModel::QUERY_ALL);

	m_view = new QListView();
	m_view->setModel(m_model);
	m_view->setViewMode(QListView::IconMode);
	m_view->setResizeMode(QListView::Adjust);
	m_view->setUniformItemSizes(true);
	m_view->setSpacing(6);
	m_view->setVerticalScrollMode(QListView::ScrollPerPixel);
	m_view->setHorizontalScrollMode(QListView::ScrollPerPixel);
	m_view->setSelectionMode(QListView::ExtendedSelection);
	mainlayout->addWidget(m_view);

	m_searchbox = new QLineEdit();
	m_searchbox->setPlaceholderText(tr("Search"));
	m_searchbox->setCompleter(new TagCompleter(m_gallery->database()->tags()));

	new QShortcut(QKeySequence("Ctrl+F"), m_searchbox, SLOT(setFocus()));

	mainlayout->addWidget(m_searchbox);

	connect(m_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openPicture(QModelIndex)));
	connect(m_searchbox, SIGNAL(returnPressed()), this, SLOT(updateQuery()));
}

void BrowserWidget::openPicture(const QModelIndex& index)
{
	const Picture *pic = m_model->pictureAt(index.row());
	if(pic!=0)
		emit pictureSelected(*pic);
}

void BrowserWidget::updateQuery()
{
	QString search = m_searchbox->text().trimmed().toLower();
	bool ok = true;

	// Special cases
	if(search.length()==0) {
		// No filter
		m_model->setQuery(ThumbnailModel::QUERY_ALL);
	} else if(search.at(0)==':') {
		// Special queries
		if(search == ":untagged")
			m_model->setQuery(ThumbnailModel::QUERY_UNTAGGED);
		else if(search == ":new")
			m_model->setQuery(ThumbnailModel::QUERY_NEW);
		else if(search == ":hidden")
			m_model->setQuery(ThumbnailModel::QUERY_HIDDEN);
		else
			ok = false;
	} else {
		// Normal query
		TagQuery query(search);
		query.init(m_gallery->database()->tags());
		ok = !query.isError();
		if(ok)
			m_model->setQuery(query);
		else
			qDebug() << "BAD QUERY:" << query.errorMessage();
	}

	if(ok) {
		m_searchbox->setPalette(QPalette());
	} else {
		QPalette palette;
		palette.setColor(QPalette::Text, Qt::red);
		m_searchbox->setPalette(palette);
	}

}

void BrowserWidget::refreshQuery()
{
	m_model->refreshQuery();
}

void BrowserWidget::uncacheSelected()
{
	m_model->uncache(getCurrentSelection());
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

QVector<int> BrowserWidget::getSelection() const
{
	QModelIndexList sel = m_view->selectionModel()->selectedRows();
	QVector<int> selection;
	foreach(QModelIndex i, sel) {
		selection.append(i.row());
	}
	return selection;
}

void BrowserWidget::selectPicture(int index)
{
	m_view->selectionModel()->setCurrentIndex(m_model->index(index), QItemSelectionModel::SelectCurrent);
}
