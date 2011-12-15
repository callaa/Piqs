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

/**
 * \brief Results of a tag matching operation
 *
 * Usually, you want to use the match(TagIdSet) function that simply returns a boolean value.
 * However, when you need extra information about how the query matched, use the query(TagIdSet) method
 * that returns an instance of this class.
 */
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

	/**
	 * \brief Does this query contain an error?
	 *
	 * An error is typically a syntax error encountered during parsing.
	 * \return true if query is invalid
	 */
	bool isError() const;

	/**
	 * \brief Get the query error message
	 * \return error message
	 * \pre isError() == true
	 */
	const QString& errorMessage() const;

	/**
	 * \brief Look up tag IDs in preparation for query matching.
	 *
	 * \param tags tag collection
	 */
	void init(const Tags *tags);

	/**
	 * \brief Look up or create tags in preparation for analysis query
	 *
	 * Unlike init(Tags), this function creates the missing tags. This initializer
	 * is used together with query(TagIdSet) when performing tag inference.
	 * \param tags tag collection
	 */
	void queryInit(Tags *tags);

	/**
	 * \brief Get a list of tag IDs mentioned in the query
	 *
	 * The list can be used to build a shortlist of interesting tags for further matching.
	 * The returned list is a string list for query building convenience.
	 * \return list of tags used in the query (excluding negated tags)
	 */
	QStringList mentionedTagIds() const;

	/**
	 * \brief Is this a "trivial" query.
	 * A query if considered trivial if it satisfies one of the following criteria:
	 * - The query consists of a single (possibly negated) tag
	 * - The query consists of more than one disjoint tag (OR query). The tags may not be in sets. Negations
	 *   are allowed, but if more than one is used, all pictures will be returned.
	 *
	 * A trivial query can be matched simply and efficiently in pure SQL.
	 * \return true if query is simple enough to be converted into SQL
	 */
	bool isTrivial() const;

	/**
	 * \brief Convert this query to SQL.
	 *
	 * This is only implemented for simple queries, so check that isTrivial() returns true before calling this.
	 * It should be possible to perform all queries in pure SQL, but for complex queries the resulting SQL code would be monstrous.
	 *
	 * If the query is nontrivial, you must iterate through each tag id set and match them individually using match(TagIdSet).
	 * You can use mentionedTagIds() to generate a shortlist so you don't need to go through the entire database.
	 *
	 * The query returns a list of picture IDs.
	 * \pre isTrivial() == true
	 */
	QString toSql() const;

	/**
	 * \brief Does this query match against the given tag ID set?
	 *
	 * \param tags the tag set to match this query against
	 * \return true if query matches the tag set
	 */
	bool match(const TagIdSet &tags) const;

	/**
	 * \brief Match against the given ID set and return details
	 *
	 * The matching algorithm is the same as in match(TagIdSet), except more information
	 * about the match is recorded. This information can be used for tag inference.
	 * \param tags the tag set to match this query against
	 * \return match results
	 */
	TagMatchResults query(const TagIdSet &tags) const;

private:
	TagQueryPrivate *m_p;
};

QDebug operator<<(QDebug dbg, const TagQuery &q);

#endif // TAGQUERY_H
