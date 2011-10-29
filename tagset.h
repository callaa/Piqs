#ifndef TAGSET_H
#define TAGSET_H

#include <QStringList>
#include <QVector>

class Database;
class QSqlQuery;

/**
  \brief The collection of tags on an image
  */
class TagSet
{
public:
    TagSet();

	//! Parse a tag string
	static TagSet parse(const QString& tagstring);

	//! Get the tagset for a picture
	static TagSet getForPicture(const Database *db, int picid);

	/**
	  \brief Get the number of tag sets.

	  \return tag set count
	  */
	int sets() const { return m_sets.size()-1; }

	//! Get the tags in the given set
	QStringList tags(int set) const;

	//! Get the tag set in the same format as accepted by parse(QString)
	QString toString() const;

	//! Save the tags for a picture
	void save(const Database *db, int picture);

private:
	QList<QStringList> m_sets;
};

typedef QVector<int> TagIdVector;

/**
  A variant of TagSet except for tag IDs. This is for tag query matching.
  */
class TagIdSet
{
public:
	TagIdSet();

	//! Get the tagset for a picture from an open result set
	static TagIdSet getFromResults(QSqlQuery &query);

	/**
	  \brief Get the number of tag sets.

	  \return tag set count
	  */
	int sets() const { return m_sets.size()-1; }

	//! Get the tags in the given set
	const TagIdVector& tags(int index) const { return m_sets.at(index); }

	/**
	  \brief Get the ID of the picture to which these tags belong to

	  \return picture Id or -1 if not valid
	  */
	int pictureId() const { return m_picid; }

private:
	QVector<TagIdVector> m_sets;
	int m_picid;
};

#endif // TAGSET_H
