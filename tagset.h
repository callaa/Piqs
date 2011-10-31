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

	//! Construct from a tag set
	TagIdSet(const TagSet &tagset, const Database *db, int pictureid=-1);

	//! Get the tagset for a picture from an open result set
	static TagIdSet getFromResults(QSqlQuery &query);

	/**
	  \brief Get the number of tag sets.

	  \return tag set count
	  */
	int sets() const { return m_sets.size()-1; }

	//! Get the total number of tags in the set
	int totalCount() const;

	//! Get the tags in the given set
	const TagIdVector& tags(int index) const { return m_sets.at(index); }

	//! Insert tags to the given set, ignoring duplicates
	void insertTags(const TagIdVector& tags, int set);

	//! Insert a new tag set, unless a set containing the same values already exists
	void insertSet(const TagIdVector& tags);

	/**
	  \brief Get the ID of the picture to which these tags belong to

	  \return picture Id or -1 if not valid
	  */
	int pictureId() const { return m_picid; }

	//! Save the tags
	void save(const Database *db);

private:
	int m_picid;
	QVector<TagIdVector> m_sets;
};

#endif // TAGSET_H
