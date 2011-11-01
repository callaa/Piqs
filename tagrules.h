#ifndef TAGIMPLICATIONS_H
#define TAGIMPLICATIONS_H

#include "tagquery.h"
#include "tagset.h"

//! A single tag implication rule
struct TagImplication {
	TagImplication(const TagQuery& rule, const TagSet& tags)
		: query(rule), tagset(tags)
	{
	}

	//! The query for matching a tag set
	TagQuery query;

	//! The tags to be added to the tag set if it matches
	TagSet tagset;

	TagIdSet idset;
};

//! Unparsed tag implication/tag alias rule
struct TagRuleExpression {
	enum Type {ALIAS, IMPLICATION};

	QString rule;
	QString tagset;
	Type type;
};

//! Tag rule file parse exception
struct TagRuleParseException {
	TagRuleParseException(const QString& filename, int linenumber, const QString& msg)
			: file(filename), line(linenumber), message(msg) { }

	QString file;
	int line;
	QString message;

};

class QDir;

class TagImplications
{
public:
    TagImplications();
	static TagImplications load(Database *database);

	/**
	  \brief parse a textual tag rule file.

	  Parses tag aliases and implications from a rule file. The returned objects
	  can be saved in the database as tag aliases and rules.

	  \param file the name of the current file. This is shown only in the parse error exception
	  \param src the IO device to read (i.e. a text file or qbuffer)
	  \param rootdir the directory from which to search included files
	  \throws TagRuleParseException
	  \returns list of tag rule expressions.
	  */
	static QList<TagRuleExpression> parseRuleFile(const QString& file, QIODevice &src, const QDir& rootdir);

	//! Apply tag implications to the given tag set
	void apply(TagIdSet &tagset) const;

private:
	void applyRule(TagIdSet &tagset, const TagMatchResults &results, const TagIdSet& newtags) const;

	QList<TagImplication> m_rules;
};

#endif // TAGIMPLICATIONS_H
