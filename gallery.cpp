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
	: QObject(parent), m_root(findRootGallery(root)), m_database(0), m_ok(false)
{
	// TODO error handling
	if(!m_root.exists(METADIR))
		if(!m_root.mkdir(METADIR))
			return;

	m_metadir = m_root;
	m_metadir.cd(METADIR);

	m_database = new Database(m_metadir, this);

	m_ok = true;
}

bool Gallery::isOk() const
{
	return m_ok;
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
	if(m_database!=0) {
		QString dbname = m_database->name();
		delete m_database;
		QSqlDatabase::removeDatabase(dbname);
	}
}

int Gallery::totalCount() const
{
	QSqlQuery q("SELECT COUNT(picid) FROM picture", m_database->get());
	if(!q.next())
		return -1;
	return q.value(0).toInt();
}
