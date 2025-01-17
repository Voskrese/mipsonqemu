/*
Launchy: Application Launcher
Copyright (C) 2007  Josh Karlin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>

//#ifdef Q_WS_WIN
#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
//#endif

#include <QSet>
#include <QDir>
#include <QUrl>
#include <QList>
#include <QFile>
#include <QIcon>
#include <QHash>
#include <qDebug>
#include <QtSql>
#include <QMenu>
#include <QString>
#include <QBuffer>
#include <QTimer>
#include <QWidget>
#include <QThread>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QBitArray>
#include <QSettings>
#include <QDateTime>
#include <QSqlQuery>
#include <QStringList>
#include <QSqlRecord>
#include <QFileDialog>
#include <QResource>
#include <QTextCodec>
#include <QEventLoop>
#include <QSemaphore>
#include <QTextStream>
#include <QDataStream>
#include <QTimerEvent>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QWaitCondition>
#include <QCoreApplication>
#include <QContextMenuEvent>

#include <QXmlStreamReader>
#include <QCryptographicHash>
#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include <boost/shared_ptr.hpp>

using namespace boost;


extern QString gNativeSep;
extern QString gSearchTxt;
extern QString gIeFavPath;

extern QSemaphore gSemaphore;

extern QWidget* gMainWidget;
extern QSettings* gSettings;

struct Directory
{
	Directory():indexDirs(false), indexExe(false), depth(100)
	{

	}
	Directory(QString n, QStringList t, bool d, bool e, int dep):indexDirs(d), indexExe(e), name(n), types(t), depth(dep)
	{
	}
	bool indexDirs;
	bool indexExe;
	QString name;
	QStringList types;
	int depth;
	int index;
};
#endif
