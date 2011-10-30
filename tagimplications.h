#ifndef TAGIMPLICATIONS_H
#define TAGIMPLICATIONS_H

#include "tagquery.h"
#include "tagset.h"

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

class TagImplications
{
public:
    TagImplications();
	static TagImplications load(const Database *database);

	//! Apply tag implications to the given tag set
	void apply(TagIdSet &tagset) const;

private:
	void applyRule(TagIdSet &tagset, const TagMatchResults &results, const TagIdSet& newtags) const;

	QList<TagImplication> m_rules;
};

#endif // TAGIMPLICATIONS_H
