#include <QRegExp>

#include "tagvalidator.h"
#include "tagset.h"

static const QChar BADCHARS[] = {'!', '(', ')' };

TagValidator::TagValidator(QObject *parent) :
    QValidator(parent)
{
}

QValidator::State TagValidator::validate(QString& input, int &pos) const
{
	QString str = input.trimmed();

	if(str.length()==0)
		return Acceptable;


	TagSet ts = TagSet::parse(str);
	for(int i=0;i<=ts.sets();++i) {
		foreach(const QString& tag, ts.tags(i)) {
			// A tag name cannot start with ":" because that is reserved for
			// pseudo tags
			if(tag.at(0)==':')
				return Invalid;

			// A tag name cannot contain any of the separator characters
			for(int j=0;j<tag.length();++j) {
				QChar chr = tag.at(j);
				for(unsigned int k=0;k<sizeof(BADCHARS);++k) {
					if(chr==BADCHARS[k])
						return Invalid;
				}
			}
		}
	}

	return Acceptable;
}
