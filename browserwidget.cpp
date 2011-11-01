#include <QListView>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QAction>
#include <QMenu>

#include "browserwidget.h"
#include "gallery.h"
#include "thumbnailmodel.h"
#include "tagquery.h"
#include "tagcompleter.h"
#include "imageinfodialog.h"

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

	m_viewctxmenu = new QMenu(this);
	QAction *act_hide = new QAction(tr("Hide"), this);
	QAction *act_show = new QAction(tr("Show"), this);
	QAction *act_info = new QAction(tr("Information"), this);

	m_viewctxmenu->addAction(act_hide);
	m_viewctxmenu->addAction(act_show);
	m_viewctxmenu->addSeparator();
	m_viewctxmenu->addAction(act_info);

	connect(act_hide, SIGNAL(triggered()), this, SLOT(picSelectedHide()));
	connect(act_show, SIGNAL(triggered()), this, SLOT(picSelectedShow()));
	connect(act_info, SIGNAL(triggered()), this, SLOT(picSelectedInfo()));

	m_view->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(pictureContextMenu(QPoint)));

	m_searchbox = new QLineEdit();
	m_searchbox->setPlaceholderText(tr("Search"));
	m_searchbox->setCompleter(new TagCompleter(m_gallery->database()->tags()));

	new QShortcut(QKeySequence("Ctrl+F"), m_searchbox, SLOT(setFocus()));

	mainlayout->addWidget(m_searchbox);

	connect(m_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openPicture(QModelIndex)));
	connect(m_searchbox, SIGNAL(returnPressed()), this, SLOT(updateQuery()));
}

void BrowserWidget::pictureContextMenu(const QPoint& point)
{
	QModelIndex index = m_view->indexAt(point);
	if(index.isValid()) {
		const Picture *p = getPictureAt(getCurrentSelection());
		if(p->isHidden()) {
			m_viewctxmenu->actions()[0]->setVisible(false);
			m_viewctxmenu->actions()[1]->setVisible(true);
		} else {
			m_viewctxmenu->actions()[0]->setVisible(true);
			m_viewctxmenu->actions()[1]->setVisible(false);
		}
		m_viewctxmenu->popup(m_view->mapToGlobal(point));
	}
}

void BrowserWidget::picSelectedInfo()
{

	qDebug() << getCurrentSelection();
	const Picture *pic = getPictureAt(getCurrentSelection());
	ImageInfoDialog *info = new ImageInfoDialog(pic->fullpath(m_gallery));
	info->setAttribute(Qt::WA_DeleteOnClose, true);
	info->show();
}

void BrowserWidget::picSelectedShow()
{

}

void BrowserWidget::picSelectedHide()
{

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
