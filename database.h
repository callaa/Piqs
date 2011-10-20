#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>

class QDir;
class Picture;

class Database : public QObject
{
    Q_OBJECT
public:
	explicit Database(const QDir& metadir, QObject *parent = 0);

	//! Is the database opened succesfully?
	bool isOpen() const { return _open; }

	//! Synchronize gallery with database
	void syncPictures(const QList<Picture>& pictures);
signals:

public slots:

private:
	static int dbindex;

	QString _dbname;
	bool _open;
};

#endif // DATABASE_H
