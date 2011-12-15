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
#include <QStack>
#include <QSharedPointer>
#include <QSet>

#include "tagquery.h"
#include "util.h"
#include "database.h"
#include "tagset.h"
#include "tags.h"

class ParseException {
public:
	ParseException(const QString& msg) : message(msg) { }

	const QString message;
};

class TagQueryNode {
public:
	virtual ~TagQueryNode() { }

	virtual int precedence() const { return 0; }

	virtual void debug(QDebug &dbg) const = 0;

	virtual void init(const Tags *tags) = 0;

	virtual void queryInit(Tags *tags) = 0;

	virtual void gatherTagIds(QSet<int>& list, bool negate) const = 0;

	/**
	 * \brief Check if this is a trivial query
	 * \param seenAnd and query detected
	 * \param seenOr or query detected
	 */
	virtual bool isTrivial() const = 0;

	//! Check if this query contains any tag set operators
	virtual bool hasSets() const { return false; }

	/**
	  \param tags the tag set to match again
	  \param limitset if nonzero, match only against the tags in that set. Otherwise, tags from all sets are matched.
	  \param limitmask bitmask of sets that have been matched already
	  */
	virtual bool match(const TagIdSet &tags, int limitset, long &limitmask) const = 0;

	/**
	  A matcher that records details about how the query matches
	  \param tags the tag set to match against
	  \param limitmask if >=0, math only against the tags in that set.
	  \param results query results are stored here
	  */
	virtual bool match(const TagIdSet &tags, int limitset, TagMatchResults &results) const = 0;
};

// Tag node
class TagQueryLeafNode : public TagQueryNode
{
public:
	TagQueryLeafNode(const QString& value) : m_value(value) { }

	const QString& value() const { return m_value; }

	void init(const Tags *tags)
	{
		m_id = tags->get(m_value);
		/*if(m_id<=0)
			throw ParseException("No such tag: " + m_value);
			*/
	}

	bool isTrivial() const
	{
		return true;
	}

	void queryInit(Tags *tags)
	{
		// Tags starting with ":" have special meaning
		if(m_value.at(0)==':')
			m_id = -1;
		else
			m_id = tags->getOrCreate(m_value);
	}

	void gatherTagIds(QSet<int>& list, bool negate) const {
		if(m_id>0 && !negate)
			list.insert(m_id);
	}

	bool match(const TagIdSet &tags, int limitset, long &limitmask) const
	{
		Q_UNUSED(limitmask);

		if(m_id<=0)
			return false;

		if(limitset<0) {
			for(int i=0;i<=tags.sets();++i)
				if(tags.tags(i).contains(m_id))
					return true;
			return false;
		} else {
			return tags.tags(limitset).contains(m_id);
		}
	}

	bool match(const TagIdSet &tags, int limitset, TagMatchResults &results) const
	{
		Q_UNUSED(results);
		if(m_id<=0) {
			// Some special pseudo tags are recognized here
			if(m_value == ":any")
				return true;
			return false;
		} else {
			// Otherwise, match tags as usual
			long unused=0;
			return match(tags, limitset, unused);
		}
	}

	void debug(QDebug &dbg) const
	{
		dbg << m_value;
	}
private:
	QString m_value;
	int m_id;
};

class TagQueryBinaryNode : public TagQueryNode
{
public:

	TagQueryBinaryNode() : m_left(0), m_right(0) { }

	~TagQueryBinaryNode() {
		delete m_left;
		delete m_right;
	}

	void init(const Tags *database)
	{
		if(m_left==0 || m_right==0)
			throw ParseException("BUG: Uninitialized binary operator node");
		m_left->init(database);
		m_right->init(database);
	}

	void queryInit(Tags *database)
	{
		if(m_left==0 || m_right==0)
			throw ParseException("BUG: Uninitialized binary operator node");
		m_left->queryInit(database);
		m_right->queryInit(database);
	}

	void gatherTagIds(QSet<int>& list, bool negate) const {
		m_left->gatherTagIds(list, negate);
		m_right->gatherTagIds(list, negate);
	}

	void setLeft(TagQueryNode *node) {
		m_left = node;
	}

	void setRight(TagQueryNode *node) {
		m_right = node;
	}

	bool hasSets() const {
		return m_left->hasSets() || m_right->hasSets();
	}

protected:
	TagQueryNode *m_left, *m_right;
};

class TagQueryUnaryNode : public TagQueryNode
{
public:
	~TagQueryUnaryNode() {
		delete m_node;
	}

	void init(const Tags *database)
	{
		if(m_node==0)
			throw ParseException("BUG: Uninitialized unary operator node");
		m_node->init(database);
	}

	void queryInit(Tags *database)
	{
		if(m_node==0)
			throw ParseException("BUG: Uninitialized unary operator node");
		m_node->queryInit(database);
	}

	void gatherTagIds(QSet<int>& list, bool negate) const {
		m_node->gatherTagIds(list, negate);
	}

	void setNode(TagQueryNode *node) { m_node = node; }

	bool hasSets() const {
		return m_node->hasSets();
	}

protected:
	TagQueryNode *m_node;
};


class TagQueryAndNode : public TagQueryBinaryNode
{
public:
	void debug(QDebug &dbg) const
	{
		dbg << "(";
		m_left->debug(dbg);
		dbg << "AND";
		m_right->debug(dbg);
		dbg << ")";
	}

	bool isTrivial() const
	{
		// AND queries are difficult, because a single tag can appear multiple
		// times in an image (in different sets of course.)
		// This makes it hard to match in pure SQL.
		return false;
	}

	bool match(const TagIdSet &tags, int limitset, long &limitmask) const
	{
		return m_left->match(tags, limitset, limitmask) && m_right->match(tags, limitset, limitmask);
	}

	bool match(const TagIdSet &tags, int limitset, TagMatchResults &results) const
	{
		return m_left->match(tags, limitset, results) && m_right->match(tags, limitset, results);
	}
};

class TagQueryOrNode : public TagQueryBinaryNode
{
public:
	int precedence() const { return 1; }

	void debug(QDebug &dbg) const
	{
		dbg << "(";
		m_left->debug(dbg);
		dbg << "OR";
		m_right->debug(dbg);
		dbg << ")";

	}

	bool isTrivial() const
	{
		return m_left->isTrivial() && m_right->isTrivial();
	}

	bool match(const TagIdSet &tags, int limitset, long &limitmask) const
	{
		return m_left->match(tags, limitset, limitmask) || m_right->match(tags, limitset, limitmask);
	}

	bool match(const TagIdSet &tags, int limitset, TagMatchResults &results) const
	{
		return m_left->match(tags, limitset, results) || m_right->match(tags, limitset, results);
	}
};

class TagQueryNotNode : public TagQueryUnaryNode
{
public:
	int precedence() const { return 2; }

	void debug(QDebug &dbg) const
	{
		dbg << "NOT (";
		m_node->debug(dbg);
		dbg << ")";
	}

	bool isTrivial() const
	{
		// Negating individual tags is easy
		if(dynamic_cast<TagQueryLeafNode*>(m_node))
			return true;
		return false;
	}

	void gatherTagIds(QSet<int>& list, bool negate) const {
		m_node->gatherTagIds(list, !negate);
	}

	bool match(const TagIdSet &tags, int limitset, long &limitmask) const
	{
		return !m_node->match(tags, limitset, limitmask);
	}

	bool match(const TagIdSet &tags, int limitset, TagMatchResults &results) const
	{
		return !m_node->match(tags, limitset, results);
	}
};

class TagQuerySetNode : public TagQueryUnaryNode
{
public:
	void debug(QDebug &dbg) const
	{
		dbg << "[";
		m_node->debug(dbg);
		dbg << "]";
	}

	bool isTrivial() const
	{
		return false;
	}

	bool match(const TagIdSet &tags, int limitset, long &limitmask) const
	{
		Q_UNUSED(limitset); // Although the grammar allows nested tag sets, other limitations prevent their use.

		for(int i=1;i<=tags.sets();++i) {
			if(! (limitmask & (1<<i))) {
				if(m_node->match(tags, i, limitmask)) {
					limitmask |= (1<<i);
					return  true;
				}
			}
		}
		return false;
	}

	bool match(const TagIdSet &tags, int limitset, TagMatchResults &results) const
	{
		Q_UNUSED(limitset); // Although the grammar allows nested tag sets, other limitations prevent their use.
		for(int i=1;i<=tags.sets();++i) {
			if(!results.tagsets.contains(i)) {
				bool m = m_node->match(tags, i, results);
				if(m) {
					results.tagsets.append(i);
					return true;
				}
			}
		}
		return false;
	}

	bool hasSets() const {
		return true;
	}
};

class TagQueryParser {
public:
	TagQueryParser(const QStringList& tokens)
		: m_tokens(tokens), m_pos(0)
	{
	}

	~TagQueryParser();

	TagQueryNode *parse();

private:
	void parseE();
	void parseP();

	void popOperator();
	void pushOperator(TagQueryNode *node);

	QString next() const {
		if(m_pos>=m_tokens.length())
			return QString();
		return m_tokens.at(m_pos);
	}
	void expect(const QString& token);

	void consume() {
		if(m_pos < m_tokens.size())
			++m_pos;
	}

	const QStringList m_tokens;
	int m_pos;
	QStack<TagQueryNode*> m_operators;
	QStack<TagQueryNode*> m_operands;
};

TagQueryNode *TagQueryParser::parse()
{
	m_operators.push(0);
	parseE();
	if(m_pos < m_tokens.length())
		throw ParseException(QString("Parsing ended at token ") + QString::number(m_pos) + QString(" of ") + QString::number(m_tokens.size()));

	return m_operands.pop();

}

TagQueryParser::~TagQueryParser()
{
	while(m_operands.size()>0)
		delete m_operands.pop();

	while(m_operators.size()>0)
		delete m_operators.pop();
}

void TagQueryParser::parseE()
{
	parseP();
	while(next() == "," || next() == "|") {
		if(next() == ",")
			pushOperator(new TagQueryAndNode());
		else
			pushOperator(new TagQueryOrNode());
		consume();
		parseP();
	}
	while(m_operators.last() != 0) {
		popOperator();
	}

}

void TagQueryParser::parseP() {
	if(next() == "(") {
		consume();
		m_operators.push(0);
		parseE();
		expect(")");
		delete m_operators.pop();
	} else if(next() == "[") {
		consume();
		m_operators.push(0);
		pushOperator(new TagQuerySetNode());
		parseE();
		expect("]");
		delete m_operators.pop();
	} else if(next() == "!") {
		pushOperator(new TagQueryNotNode());
		consume();
		parseP();
	} else {
		m_operands.push(new TagQueryLeafNode(next()));
		consume();
	}
}

void TagQueryParser::popOperator() {
	TagQueryNode *node = m_operators.pop();
	TagQueryBinaryNode *bin = dynamic_cast<TagQueryBinaryNode*>(node);
	if(bin!=0) {
		bin->setRight(m_operands.pop());
		bin->setLeft(m_operands.pop());
		m_operands.push(bin);
	} else {
		TagQueryUnaryNode *una = static_cast<TagQueryUnaryNode*>(node);
		una->setNode(m_operands.pop());
		m_operands.push(una);
	}
}

void TagQueryParser::pushOperator(TagQueryNode *node) {
	while(m_operators.last()!=0 && m_operators.last()->precedence() > node->precedence()) {
		popOperator();
	}

	m_operators.push(node);
}

void TagQueryParser::expect(const QString& token) {
	if(token == next())
		consume();
	else
		throw ParseException("Expected token \"" + token + "\" but got \"" + next() + "\"");
}

struct TagQueryPrivate {
	QSharedPointer<TagQueryNode> node;
	QString error;
};

TagQuery::TagQuery(const QString& query)
	: m_p(new TagQueryPrivate())
{
	QStringList tokens = Util::tokenize(query, "()[]!,|", true);

	// TODO add naive parenthesis balancing for user friendliness
	try {
		m_p->node = QSharedPointer<TagQueryNode>(TagQueryParser(tokens).parse());
	} catch(const ParseException& e) {
		m_p->error = e.message;
	}

}

TagQuery::TagQuery()
	: m_p(new TagQueryPrivate())
{
}

TagQuery::TagQuery(const TagQuery &tq)
	: m_p(new TagQueryPrivate(*tq.m_p))
{
}

TagQuery& TagQuery::operator=(const TagQuery &tq)
{
	*m_p = *tq.m_p;
	return *this;
}

TagQuery::~TagQuery()
{
	delete m_p;
}

bool TagQuery::isError() const
{
	return m_p->error.length()>0;
}

const QString& TagQuery::errorMessage() const
{
	return m_p->error;
}

void TagQuery::init(const Tags *tags)
{
	if(m_p->node!=0) {
		try {
			m_p->node->init(tags);
		} catch(const ParseException& e) {
			m_p->error = e.message;
		}
	}
}

void TagQuery::queryInit(Tags *tags)
{
	if(m_p->node!=0) {
		try {
			m_p->node->queryInit(tags);
		} catch(const ParseException& e) {
			m_p->error = e.message;
		}
	}
}

static QStringList set2list(const QSet<int> &set)
{
	QStringList list;
	foreach(int i, set)
		list.append(QString::number(i));
	return list;
}

static QString gettag(const QSet<int> &set)
{
	Q_ASSERT(set.count()==1);
	return QString::number(*set.constBegin());
}

QStringList TagQuery::mentionedTagIds() const
{
	if(m_p->node!=0) {
		QSet<int> ids;
		m_p->node->gatherTagIds(ids, false);
		return set2list(ids);
	}
	return QStringList();
}

bool TagQuery::isTrivial() const
{
	if(m_p->node==0)
		return true;
	return m_p->node->isTrivial();
}

QString TagQuery::toSql() const
{
	if(m_p->node!=0) {
		// Gather tag IDs
		QSet<int> tags, nottags;
		m_p->node->gatherTagIds(tags, false);
		m_p->node->gatherTagIds(nottags, true);

		// Check if we are in "OR" mode. If this is a trivial query, the ! is
		// not used for groups, therefore if the query contains a | operator,
		// the root node must be an OR node.
		bool ormode = dynamic_cast<TagQueryOrNode*>(m_p->node.data()) != 0;

		if(ormode) {
			if(nottags.count()>1) {
				// Special case sanity check: ¬A ∪ ¬B = U
				return "SELECT picid FROM picture";
			}

			QString sql = "SELECT picid FROM tagmap WHERE";
			if(tags.count()>0) {
				sql = sql + " tagid IN (" + set2list(tags).join(",") + ")";
			}
			if(nottags.count()==1) {
				if(tags.count()>0)
					sql = sql + " OR picid NOT IN (SELECT picid FROM tagmap WHERE tagid=" + gettag(nottags) + ")";
				else
					sql = sql + " tagid!=" + gettag(nottags);
			}
			sql += " GROUP BY picid";
			return sql;
		} else {
			// If not in OR mode, we can match only one tag at a time
			Q_ASSERT(tags.count() + nottags.count() == 1);
			QString sql;
			if(tags.count()>0)
				sql = "SELECT picid FROM tagmap WHERE tagid=" + gettag(tags) + " GROUP BY picid";
			else
				sql = "SELECT picid FROM tagmap WHERE tagid!=" + gettag(nottags) + " GROUP BY picid";
			return sql;
		}
	} else {
		// Empty query matches everything
		return "SELECT picid FROM picture";
	}
}

/**
  This is used for query filtering
  \param tags the tag set to match against
  \return true if query matches
  */
bool TagQuery::match(const TagIdSet &tags) const
{
	if(m_p->node!=0) {
		long limitmask=0;
		return m_p->node->match(tags, -1, limitmask);
	}
	return false;
}

/**
  This is used for tag analysis in tag induction.
  \param tags the tag set to match against
  \return detailed query results
  */
TagMatchResults TagQuery::query(const TagIdSet &tags) const
{
	TagMatchResults results;
	if(m_p->node!=0) {
		if(m_p->node->hasSets()) {
			// If query has tag sets, match the usual way
			results.matchsets = true;
			if(m_p->node->match(tags, -1, results))
				results.matched = true;

		} else {
			// If it is flat, try querying the tag sets separately first
			for(int i=0;i<=tags.sets();++i) {
				if(m_p->node->match(tags, i, results)) {
					results.matched = true;
					results.tagsets.append(i);
				}
			}

			// If no matches have been found so far, try querying
			// without set borders.
			if(results.matched==false && tags.sets()>0) {
				if(m_p->node->match(tags, -1, results))
					results.matched = true;
			}
		}
	}
	return results;
}

QDebug operator<<(QDebug dbg, const TagQuery &q)
{
	dbg.space() << "QUERY:";
	if(q.isError())
		dbg.space() << q.errorMessage();
	else
		q.m_p->node->debug(dbg.space());

	return dbg.space();
}

QDebug operator<<(QDebug dbg, const TagMatchResults &r)
{
	QDebug& d = dbg.space();
	d << "RESULTS:";
	if(r.matched) {
		d << "matched=true,";
		if(r.matchsets)
			d << "tagset match order:" << r.tagsets;
		else
			d << "matching tagsets:" << r.tagsets;
	} else {
		d << "not matched";
	}

	return d;
}
