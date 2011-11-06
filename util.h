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
