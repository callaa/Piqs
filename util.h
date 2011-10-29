#ifndef UTIL_H
#define UTIL_H

#include <QStringList>

//! Various ulitity methods
class Util
{
public:
	//! Split the string at the tokens
	static QStringList tokenize(const QString& string, const QString& tokens, bool trim);
};

#endif // UTIL_H
