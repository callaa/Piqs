#include <QSqlQuery>
#include <QVariant>

#include "tagimplications.h"

#include "database.h"

TagImplications::TagImplications()
{
}

TagImplications TagImplications::load(const Database *database)
{
	TagImplications ti;
	QSqlQuery q("SELECT rule, tags FROM tagrule ORDER BY ruleorder ASC", database->get());
	while(q.next()) {
		TagQuery query = TagQuery(q.value(0).toString());
		TagSet set = TagSet::parse(q.value(1).toString());

		ti.m_rules.append(TagImplication(query, set));
	}
	q.finish();

	for(QList<TagImplication>::iterator i=ti.m_rules.begin();i!=ti.m_rules.end();++i) {
		i->query.init(database);
		i->idset = TagIdSet(i->tagset, database);
	}

	return ti;
}

void TagImplications::apply(TagIdSet &tagset) const
{
	int lastcount=0;
	while(tagset.totalCount() > lastcount) {
		lastcount = tagset.totalCount();

		foreach(TagImplication rule, m_rules) {
			TagMatchResults result = rule.query.query(tagset);
			if(result.matched) {
				applyRule(tagset, result, rule.idset);
			}
		}
	}
}

void TagImplications::applyRule(TagIdSet &tagset, const TagMatchResults &results, const TagIdSet &newtags) const
{
	if(results.matchsets) {
		// Rule contains sets, insert tags by set.

		// Zero set tags always to go zero set
		tagset.insertTags(newtags.tags(0), 0);

		// Other tags go to the sets matched by the query
		for(int i=0;i<qMin(newtags.sets(), results.tagsets.count());++i) {
			tagset.insertTags(newtags.tags(i), results.tagsets.at(i));
		}
	} else {
		// Rule does not contain set. If tagsets is empty, rule matched cross-set
		// tags and new tags should be inserted into the zero set. Also, if the
		// new tag set contains tag sets, append the new sets.
		// Otherwise, insert the (flat) new tags to listed sets.
		if(results.tagsets.isEmpty() || newtags.sets()>0) {
			tagset.insertTags(newtags.tags(0), 0);
			for(int i=1;i<=newtags.sets();++i)
				tagset.insertSet(newtags.tags(i));
		} else {
			foreach(int set, results.tagsets) {
				tagset.insertTags(newtags.tags(0), set);
			}
		}
	}
}
