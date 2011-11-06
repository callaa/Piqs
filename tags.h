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
#ifndef TAGS_H
#define TAGS_H

#include <QAbstractListModel>
#include <QStringList>
#include <QHash>

class Database;
class QCompleter;

class Tags : public QAbstractListModel
{
    Q_OBJECT
public:
	explicit Tags(Database *database);

	//! Get the ID for the given tag, creating it if it doesn't exist already
	int getOrCreate(const QString& name);

	//! Get the ID for the given tag
	int get(const QString& name) const;

	//! (Re)create the tag index tables
	void createTables(bool dropfirst=false);

	//! Reload all tags from the database, including tag aliases
	void reload();

	int rowCount(const QModelIndex &parent) const;

	QVariant data(const QModelIndex &index, int role) const;

signals:

public slots:

private:
	Database *m_database;
	QHash<QString, int> m_taghash;
	QStringList m_tags;

};

#endif // TAGS_H
