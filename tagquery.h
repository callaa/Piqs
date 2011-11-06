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
#ifndef TAGQUERY_H
#define TAGQUERY_H

#include <QVector>
#include <QDebug>
#include <QStringList>

class TagQueryPrivate;
class Tags;
class TagIdSet;

struct TagMatchResults
{
	TagMatchResults() : matched(false), matchsets(false) { }

	//! Did query match at all
	bool matched;

	//! Did the query contain tag sets
	bool matchsets;

	/**
	 \brief List of matched tag sets in the order they appear in the query

	 The interpretation of this field depends on matchsets. If matchsets is true,
	 this vector contains the indexes of the tagsets in the order they were matched.
	 The zero set is not included.
	 For example, the query [cat] would match the tag string "[dog], [mouse], [cat]" and tagsets would be {3}

	 If matchsets is false, the query was a "flat query" that doesn't specify any tag sets. The vector
	 will contain the indexes of the tag sets that match the query in its entirety. The zero set will be included.
	 If matched is true but this vector is empty, the matching tags were not found in any one tag set.
	 */
	QVector<int> tagsets;
};

QDebug operator<<(QDebug dbg, const TagMatchResults &r);

/**
  Tag query parser and matcher.

  Query language grammar:
  <pre>
  E --> E "," E
	 | E "|" E
	 | "!" E
	 | "(" E ")"
	 | v
  </pre>
  Non left recursive form:
  <pre>
  E --> P {B P}
  P --> v | "(" E ")" | U P
  B --> "," | "|"
  U --> "!"
 </pre>
 <p>
 Before using, call init() and then check if an error was reported.

 \see http://www.engr.mun.ca/~theo/Misc/exp_parsing.htm
 */
class TagQuery
{
	friend QDebug operator<<(QDebug dbg, const TagQuery &q);
public:
	TagQuery();
	TagQuery(const QString& query);
	TagQuery(const TagQuery& tq);
	TagQuery &operator=(const TagQuery& tq);
	~TagQuery();

	//! Does this query contain an error?
	bool isError() const;

	//! Get the query error message
	const QString& errorMessage() const;

	//! Look up tag IDs in preparation for query matching
	void init(const Tags *tags);

	//! Look up or create tags in preparation for analysis query
	void queryInit(Tags *tags);

	//! Get a list of tag IDs mentioned in the query (as a string list for SQL query building convenience)
	QStringList mentionedTagIds() const;

	//! Does this query match against the given tag ID set?
	bool match(const TagIdSet &tags) const;

	//! Match against the given ID set and return details
	TagMatchResults query(const TagIdSet &tags) const;

private:
	TagQueryPrivate *m_p;
};

QDebug operator<<(QDebug dbg, const TagQuery &q);

#endif // TAGQUERY_H
