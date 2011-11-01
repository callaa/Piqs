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
