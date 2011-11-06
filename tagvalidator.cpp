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
	Q_UNUSED(pos);

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
