#ifndef TAGQUERY_H
#define TAGQUERY_H

#include <QDebug>
#include <QStringList>

class TagQueryPrivate;
class Database;
class TagIdSet;

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
	TagQuery(const QString& query);
	TagQuery(const TagQuery& tq);
	TagQuery &operator=(const TagQuery& tq);
	~TagQuery();

	//! Does this query contain an error?
	bool isError() const;

	//! Get the query error message
	const QString& errorMessage() const;

	//! Look up tag IDs in preparation for query matching
	void init(const Database *database);

	//! Get a list of tag IDs mentioned in the query (as a string list for SQL query building convenience)
	QStringList mentionedTagIds() const;

	//! Does this query match against the given tag ID set?
	bool match(const TagIdSet &tags) const;

private:
	TagQueryPrivate *m_p;
};

QDebug operator<<(QDebug dbg, const TagQuery &q);

#endif // TAGQUERY_H
