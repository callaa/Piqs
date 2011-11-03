#ifndef RESCANTHREAD_H
#define RESCANTHREAD_H

#include <QThread>

class Gallery;
class QSqlQuery;
class QDir;

//! Rescan gallery directory structure in the background
class RescanThread : public QThread
{
    Q_OBJECT
public:
	explicit RescanThread(const Gallery *gallery, QObject *parent = 0);

	void run();

signals:
	void statusChanged(const QString& status);
	void filesAdded(int count);
	void foldersSearched(int count);
	void missingFound(int count);
	void dupesFound(int count);
	void movesFound(int count);

public slots:
	void abortScan();

private:
	void rescan(const QStringList& filefilter, const QString& prefix, const QDir& root, QSqlQuery& insertquery, QSqlQuery& updatequery);
	bool periodElapsed();

	const Gallery *m_gallery;

	int m_filecount;
	int m_foldercount;

	bool m_abortflag;
	qint64 m_time;

};

#endif // RESCANTHREAD_H
