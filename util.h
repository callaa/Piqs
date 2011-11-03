#ifndef UTIL_H
#define UTIL_H

#include <QStringList>

//! Various ulitity methods
class Util
{
public:
	//! Split the string at the tokens
	static QStringList tokenize(const QString& string, const QString& tokens, bool trim);

	//! Remove illegal characters from a tag name
	static QString cleanTagName(const QString& name);

	//! Calculate a file's SHA-1 hash
	static QString hashFile(const QString& path);
};

#endif // UTIL_H
