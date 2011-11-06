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
#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QHash>

class QDir;
class Picture;
class Tags;

//! Database access
class Database : public QObject
{
    Q_OBJECT
public:
	explicit Database(const QDir& metadir, QObject *parent = 0);

	~Database();

	//! Is the database opened succesfully?
	bool isOpen() const { return m_db.isOpen(); }

	//! Get the database connection
	QSqlDatabase get() const { return m_db; }

	//! Get the name of the database connection
	const QString& name() const { return m_dbname; }

	//! Escape text for use in SQL queries
	QString esc(const QString& text) const;

	Tags *tags() { return m_tags; }

	const Tags *tags() const { return m_tags; }

	//! Save a configuration value
	void saveSetting(const QString& key, const QVariant& value) const;

	//! Get a configuration value
	QVariant getSetting(const QString& key) const;

	//! Alert the user of database errors
	static void showError(const QString& message, const QSqlQuery &query);

signals:

public slots:

private:
	static int dbindex;

	QString m_dbname;
	QSqlDatabase m_db;
	Tags *m_tags;
};

#endif // DATABASE_H
