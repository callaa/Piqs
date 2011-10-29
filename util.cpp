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
