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
#ifndef TAGCOMPLETER_H
#define TAGCOMPLETER_H

#include <QCompleter>

class Tags;

/**
  A completer that understands the tag query language
  */
class TagCompleter : public QCompleter
{
    Q_OBJECT
public:
	explicit TagCompleter(Tags* tags, QObject *parent=0);

	QString pathFromIndex( const QModelIndex& index ) const;
	QStringList splitPath( const QString& path ) const;

private:
	const Tags* m_tags;

};

#endif // TAGCOMPLETER_H
