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
#ifndef TAGSET_H
#define TAGSET_H

#include <QStringList>
#include <QVector>

class Database;
class QSqlQuery;
class Tags;

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
	TagIdSet(const TagSet &tagset, Tags *tags, int pictureid=-1);

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
	void save(const Database *db, bool transaction=true);

private:
	int m_picid;
	QVector<TagIdVector> m_sets;
};

#endif // TAGSET_H
