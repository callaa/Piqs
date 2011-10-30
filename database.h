#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QHash>

class QDir;
class Picture;

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

	//! Get the ID for the given tag, creating it if it doesn't exist already
	int getOrCreateTag(const QString& name) const;

	//! Get the ID for the given tag
	int getTag(const QString& name) const;

	//! Save a configuration value
	void saveSetting(const QString& key, const QVariant& value) const;

	//! Get a configuration value
	QVariant getSetting(const QString& key) const;

	//! (Re)create the tag index tables
	void createTagIndexTables(bool dropfirst=false) const;

signals:

public slots:

private:
	static int dbindex;

	QString m_dbname;
	QSqlDatabase m_db;
};

#endif // DATABASE_H
