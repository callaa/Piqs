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
#include <QSqlQuery>
#include <QVariant>
#include <QFile>
#include <QDir>

#include "tagrules.h"

#include "database.h"

TagImplications::TagImplications()
{
}

TagImplications TagImplications::load(Database *database)
{
	TagImplications ti;

	// Read rules from the database
	QSqlQuery q("SELECT rule, tags FROM tagrule ORDER BY ruleorder ASC", database->get());
	while(q.next()) {
		TagQuery query = TagQuery(q.value(0).toString());
		TagSet set = TagSet::parse(q.value(1).toString());

		ti.m_rules.append(TagImplication(query, set));
	}
	q.finish();

	// Initialize rules and removed invalid ones
	// Ideally, there should be no bad rules in the database, but better
	// safe than sorry.
	QMutableListIterator<TagImplication> iterator(ti.m_rules);
	while(iterator.hasNext()) {
		TagImplication &i = iterator.next();
		i.query.queryInit(database->tags());
		if(i.query.isError()) {
			qDebug() << "Bad tag rule:" << i.query;
			iterator.remove();
		} else {
			i.idset = TagIdSet(i.tagset, database->tags());
		}
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
			tagset.insertTags(newtags.tags(i+1), results.tagsets.at(i));
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

enum TagParserState {
	EXPECT_RULE,
	EXPECT_SEPARATOR,
	EXPECT_NEWTAG,
	EXPECT_NEWTAGSET,
	EXPECT_ENDORTAGSET,
	EXPECT_END
};

QList<TagRuleExpression> TagImplications::parseRuleFile(const QString& file, QIODevice &src, const QDir& rootdir)
{
	QRegExp tokenseparator("=|-->");

	// A tag alias is one or more comma separated tags
	QRegExp tagaliassyntax("(?:(?!:)[^!|[\\](),]+)(?:\\s*,\\s*(?!:)[^!|[\\](),]+)*");

	QList<TagRuleExpression> expressions;
	TagRuleExpression newexpr;

	QStringList tokens;
	TagParserState state = EXPECT_RULE;
	int linenum = 0;
	while(1) {
		// Read line from source
		QByteArray byteline = src.readLine();
		if(byteline.isEmpty())
			break;
		++linenum;
		QString line = QString::fromLocal8Bit(byteline.constData()).trimmed();
		if(line.length()==0)
			continue;

		// Special commands
		if(line.startsWith("#@")) {
			if(line.startsWith("#@include ")) {
				QString filename = line.mid(9).trimmed();
				QFile incl(rootdir.absoluteFilePath(filename));
				if(!incl.open(QFile::ReadOnly|QFile::Text))
					throw TagRuleParseException(file, linenum, QObject::tr("Cannot open file %1").arg(filename));

				expressions << parseRuleFile(filename, incl, rootdir);
				continue;
			}
			throw TagRuleParseException(file, linenum, QObject::tr("Unknown command: %1").arg(line));
		}

		// Skip comments
		if(line.at(0)=='#')
			continue;

		// Tokenize line
		int pos=0;
		int next=0;
		while(pos < line.length()) {
			next = tokenseparator.indexIn(line, pos);
			if(next<0)
				next = line.length();
			if(pos<next)
				tokens << line.mid(pos, next-pos).trimmed();
			if(next<line.length())
				tokens << line.mid(next, tokenseparator.matchedLength());
			pos = next + qMax(0, tokenseparator.matchedLength());
		}

		// Parse tokens
		while(!tokens.isEmpty()) {
			QString token = tokens.takeFirst();
			switch(state) {
			case EXPECT_RULE:
				// Expect tag rule (list of tags or query, depending on expression type)
				if(token=="=" || token=="-->")
					throw TagRuleParseException(file, linenum, QObject::tr("Expected rule, got: %1").arg(token));
				newexpr.rule = token;
				state = EXPECT_SEPARATOR;
				break;
			case EXPECT_SEPARATOR:
				// Expect tag rule/tag list separator. This determines the expression type
				if(token=="=") {
					newexpr.type = TagRuleExpression::ALIAS;
					state = EXPECT_NEWTAG;
					if(!tagaliassyntax.exactMatch(newexpr.rule))
						throw TagRuleParseException(file, linenum, QObject::tr("%1 is not a valid tag alias").arg(newexpr.rule));
				} else if(token=="-->") {
					newexpr.type = TagRuleExpression::IMPLICATION;
					state = EXPECT_NEWTAGSET;
					// TODO check rule syntax
				} else {
					// Continuation of LEFT?
					QChar last = newexpr.rule.at(newexpr.rule.length()-1);
					if(last!=',' && last!='|')
						throw TagRuleParseException(file, linenum, QObject::tr("Expected = or -->, got: %1").arg(token));
					newexpr.rule += token;
				}
				break;
			case EXPECT_NEWTAG:
			case EXPECT_NEWTAGSET:
				// Expect new tag (list)
				if(token=="=" || token=="-->")
					throw TagRuleParseException(file, linenum, QObject::tr("Expected tag list, got: %1").arg(token));

				newexpr.tagset = token;
				state = (state==EXPECT_NEWTAG ? EXPECT_END : EXPECT_ENDORTAGSET);
				break;
			case EXPECT_ENDORTAGSET:
				// Continuation of RIGHT?
				if(newexpr.tagset.at(newexpr.tagset.length()-1)==',')
					newexpr.tagset += token;
				else
					throw TagRuleParseException(file, linenum, QObject::tr("Expected tag set, got: %1").arg(token));
				break;
			case EXPECT_END:
				throw TagRuleParseException(file, linenum, QObject::tr("Expected end of line after tag name, got: %1").arg(token));
			default: throw TagRuleParseException(file, linenum, QString("BUG: Unhandled state %1").arg(state));
			}
		}

		// End of line. Tagset definitions (but not single tags) are allowed to span multiple lines
		if(state==EXPECT_END || (state==EXPECT_ENDORTAGSET && newexpr.tagset.at(newexpr.tagset.length()-1)!=',')) {
			expressions << newexpr;
			state=EXPECT_RULE;
		}

	}

	if(state!=EXPECT_RULE)
		throw TagRuleParseException(file, linenum, QObject::tr("Unexpected end of file"));

	return expressions;

}
