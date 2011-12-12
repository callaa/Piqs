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
	RescanThread(const Gallery *gallery, bool quick, QObject *parent = 0);

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
	bool m_quick;

};

#endif // RESCANTHREAD_H
