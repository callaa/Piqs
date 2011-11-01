#include <QLineEdit>

#include "tagcompleter.h"
#include "tags.h"

// TODO other separator characters: ], |, !
// See http://www.mimec.org/node/304
// and http://stackoverflow.com/questions/3779720/qt-qcompleter-multiple-matches

TagCompleter::TagCompleter(Tags *tags, QObject *parent) :
	QCompleter(tags, parent), m_tags(tags)
{
	setCaseSensitivity(Qt::CaseInsensitive);
	setCompletionMode(QCompleter::InlineCompletion);
}

QString TagCompleter::pathFromIndex( const QModelIndex& index ) const
{
	QString path = QCompleter::pathFromIndex( index );

	QString text = static_cast<QLineEdit*>( widget() )->text();

	int pos = text.lastIndexOf( ',' );
	if ( pos >= 0 )
		path = text.left( pos ) + ", " + path;

	return path;
}

QStringList TagCompleter::splitPath( const QString& path ) const
{
	int pos = path.lastIndexOf( ',' ) + 1;

	while ( pos < path.length() && path.at( pos ) == QLatin1Char( ' ' ) )
		pos++;

	return QStringList( path.mid( pos ) );
}
