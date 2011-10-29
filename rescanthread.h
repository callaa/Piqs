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
	void filesAdded(int count);

public slots:
	void abortScan();

private:
	void rescan(const QStringList& filefilter, const QString& prefix, const QDir& root, QSqlQuery& query);
	const Gallery *m_gallery;
	int m_count;
	bool m_abortflag;
	qint64 m_time;

};

#endif // RESCANTHREAD_H
