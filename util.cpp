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
#include <QFile>
#include <QCryptographicHash>

#include "util.h"

/**
  Split the string at the characters listed in the token string.
  The token characters are included in the list.

  \param string the string to tokenize
  \param tokens the characters at which the string should be separated
  \param trim remove leading and trailing whitespace from each token. If true, empty tokens will not be inserted.
  \return token list
  */
QStringList Util::tokenize(const QString& string, const QString& tokenstring, bool trim)
{
	QStringList tokens;
	const int len = string.length();
	const int tokenchars = tokenstring.length();

	int start=0;
	for(int i=0;i<len;++i) {
		const QChar chr = string.at(i);

		for(int j=0;j<tokenchars;++j) {
			if(chr == tokenstring.at(j)) {
				if(start<=i) {
					if(start<i) {
						QString t = string.mid(start, i-start);
						if(trim) {
							t = t.trimmed();
							if(t.length()>0)
								tokens.append(t);
						} else {
							tokens.append(t);
						}
					}
					start=i+1;
				}
				tokens.append(QString(chr));
			}
		}
	}
	if(start<len-1) {
		QString t = string.mid(start);
		if(trim) {
			t = t.trimmed();
			if(t.length()>0)
				tokens.append(t);
		} else {
			tokens.append(t);
		}
	}

	return tokens;
}

QString Util::cleanTagName(const QString& name)
{
	static QRegExp badchars("[,|()[\\]!]");
	return name.simplified().toLower().replace(badchars, "");
}

QString Util::hashFile(const QString& path)
{
	QFile file(path);
	if(!file.open(QFile::ReadOnly))
		return QString();

	QCryptographicHash hash(QCryptographicHash::Sha1);

	char buffer[4096];
	int r;
	while((r=file.read(buffer, sizeof buffer))>0) {
		hash.addData(buffer, r);
	}

	return QString::fromLatin1(hash.result().toHex());
}
