#include <QStringList>
#include <QFileInfoList>
#include <QDebug>
#include <QImageReader>
#include <QSqlQuery>
#include <QVariant>

#include "gallery.h"
#include "database.h"

const QString Gallery::METADIR = ".piqs";

Gallery::Gallery(const QDir& root, QObject *parent)
	: QObject(parent), m_root(findRootGallery(root))
{
	// TODO error handling
	if(!m_root.exists(METADIR))
		m_root.mkdir(METADIR);
	m_metadir = m_root;
	m_metadir.cd(METADIR);

	m_database = new Database(m_metadir, this);
}

/**
 * Travel up the directory stack until a parent directory containing
 * a .piqs directory is found.
 * <p>If no such directory is found, the original directory is returned.
 */
QDir Gallery::findRootGallery(QDir dir)
{
	QDir top = dir;
	while(1) {
		qDebug() << "finding root gallery" << dir;
		if(dir.exists(METADIR))
			return dir;
		else if(!dir.cdUp())
			break;
	}
	return top;
}


Gallery::~Gallery()
{
	// Explicitly delete the database here to release the associated connection.
	QString dbname = m_database->name();
	delete m_database;
	QSqlDatabase::removeDatabase(dbname);
}

int Gallery::totalCount() const
{
	QSqlQuery q("SELECT COUNT(*) FROM picture", m_database->get());
	if(!q.next())
		return -1;
	return q.value(0).toInt();
}
