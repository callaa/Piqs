#ifndef PICTURE_H
#define PICTURE_H

#include <QString>
#include <QMetaType>

#include "tagset.h"

class Gallery;
class Database;

class Picture
{
public:
	Picture();
	Picture(int id, const QString& filename, bool hidden, const QString& title, const QString& tags, int rotation, const QString& hash);

	//! Get the internal ID of this picture
	int id() const { return m_id; }

	//! Has this picture been hidden?
	bool isHidden() const { return m_hidden; }

	//! Get the filename relative to gallery root
	QString relativeName() const { return m_relativename; }

	//! Get the full path to this file
	QString fullpath(const Gallery *gallery) const;

	//! Get the picture title
	QString title() const { return m_title; }

	//! Get the tag string (user inputted tags)
	QString tagString() const { return m_tags; }

	//! Get the hash of the image file contents
	QString hash() const { return m_hash; }

	//! Get the picture rotation angle in degrees
	int rotation() const { return m_rotation; }

	//! Hide or show the picture
	void setHidden(Database *db, bool hidden);

	//! Delete this file from the database and the file system
	void deleteFile(Gallery *gallery);

	//! Change picture tag string and save changes to the database.
	void saveTags(Database *db, const QString& tags);

	//! Change picture title string and save changes to the database.
	void saveTitle(const Database *db, const QString& newtitle);

private:
	int m_id;
	QString m_relativename;
	bool m_hidden;
	QString m_title;
	QString m_tags;
	int m_rotation;
	QString m_hash;
};

Q_DECLARE_METATYPE(Picture)

#endif // PICTURE_H
