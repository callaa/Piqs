#include <QDebug>
#include <QSqlQuery>
#include <QVariant>

#include "database.h"
#include "tagset.h"
#include "util.h"

TagSet::TagSet()
{
	// The first item is the "zero set" that holds all the ungrouped tags
	m_sets.append(QStringList());
}

QStringList TagSet::tags(int set) const
{
	Q_ASSERT(set>=0 && set<m_sets.size());
	return m_sets.at(set);
}

/**
  Tags are separated with commas and may be grouped with []
  This is a very lenient parser.
  */
TagSet TagSet::parse(const QString& tagstring)
{
	TagSet tags;

	QStringList tokens = Util::tokenize(tagstring, ",[]", true);

	int set=0;
	bool setopen=false;

	foreach(const QString& token, tokens) {
		if(token==",") {
			// Tag separator, ignore
			continue;
		} else if(token=="[") {
			// Begin tag set
			if(!setopen) {
				++set;
				tags.m_sets.append(QStringList());
				setopen = true;
			}
		} else if(token=="]") {
			// Close tag set
			setopen = false;
		} else {
			// A tag
			if(setopen) {
				tags.m_sets[set].append(token);
			} else {
				tags.m_sets[0].append(token);
			}
		}
	}

	return tags;
}

QString TagSet::toString() const
{
	QString str;
	str.append(m_sets.at(0).join(", "));

	for(int i=1;i<m_sets.length();++i) {
		if(i>1 || m_sets.at(0).length()>0)
			str.append(", ");
		str.append('[');
		str.append(m_sets.at(i).join(", "));
		str.append(']');
	}

	return str;
}

void TagSet::save(const Database *db, int picture)
{
	// Get tag IDs
	QVector<int> tags[m_sets.length()];
	for(int i=0;i<m_sets.length();++i) {
		foreach(const QString& tag, m_sets.at(i)) {
			tags[i].append(db->getOrCreateTag(tag));
		}
	}

	// Clear out old tag associations
	db->get().transaction();
	QSqlQuery q(db->get());
	q.exec("DELETE FROM tagmap WHERE picid=" + QString::number(picture));

	// Insert new ones
	q.prepare("INSERT INTO tagmap (picid, tagid, tagset) VALUES (?, ?, ?)");
	q.bindValue(0, picture);

	for(int set=0;set<m_sets.length();++set) {
		q.bindValue(2, set);
		foreach(int tag, tags[set]) {
			q.bindValue(1, tag);
			q.exec();
		}
	}

	db->get().commit();
}

TagSet TagSet::getForPicture(const Database *db, int picid)
{
	QSqlQuery q(db->get());
	q.setForwardOnly(true);
	q.prepare("SELECT tag, tagset FROM tagmap JOIN tag USING (tagid) WHERE picid=? ORDER BY picid, tagset ASC");
	q.addBindValue(picid);
	q.exec();

	TagSet tags;
	int lastset=0;
	while(q.next()) {
		QString tag = q.value(0).toString();
		int set = q.value(1).toInt();
		if(set!=lastset) {
			// Note. Normally the tag sets numbers should be contiguous, but this may not be the case
			// if someone has manually altered the database.
			lastset = set;
			tags.m_sets.append(QStringList());
		}
		tags.m_sets.last().append(tag);
	}

	return tags;
}

TagIdSet::TagIdSet()
	: m_picid(-1)
{
}

/**
  The result set record should be the following:
  <ol>
  <li>picture id
  <li>tag id
  <li>tag set #
  </ol>
  The results should be sorted in ascending order by picture id and tag set #
  */
TagIdSet TagIdSet::getFromResults(QSqlQuery &query)
{
	TagIdSet tags;
	if(query.isValid()) {
		tags.m_sets.append(TagIdVector());
		tags.m_picid = query.value(0).toInt();
		int lastset = 0;
		do {
			if(query.value(0).toInt() != tags.m_picid)
				break;

			int set = query.value(2).toInt();
			if(set>lastset) {
				lastset = set;
				tags.m_sets.append(TagIdVector());
			}

			tags.m_sets.last().append(query.value(1).toInt());
		} while(query.next());
	}

	return tags;
}
