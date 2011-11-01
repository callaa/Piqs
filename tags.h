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
