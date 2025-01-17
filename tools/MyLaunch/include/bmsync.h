#ifndef BOOKMARK_SYNC_H
#define BOOKMARK_SYNC_H

#include <config.h>
#include <globals.h>
#include <bmnet.h>
#include <bmMerge.h>
#include <bmpost.h>


#define BM_EQUAL 1
#define BM_MODIFY 2
#define BM_DELETE 3
#define BM_DIFFERENT 4
#define BM_ADD 5

#define MERGE_FROM_SERVER 0
#define MERGE_FROM_LOCAL 1

#define ACTION_ITEM_DELETE 0
#define ACTION_ITEM_ADD 1

#define MERGE_TYPE_EQUAL_UNDO 0
#define MERGE_TYPE_EQUAL_ADD 1

#define NAME_IS_FILE 1
#define NAME_IS_DIR 2

#define	POST_HTTP_TYPE_TESTACCOUNT  1
#define POST_HTTP_TYPE_HANDLE_ITEM 2


#if defined(BOOKMARK_SYNC_DLL)
#define BOOKMARK_SYNC_CLASS_EXPORT __declspec(dllexport)
#else
#define BOOKMARK_SYNC_CLASS_EXPORT __declspec(dllimport)
#endif

enum{
	SYNC_DO_BOOKMARK=0,
	SYNC_DO_TESTACCOUNT,
	SYNC_DO_DIGG
};

class BOOKMARK_SYNC_CLASS_EXPORT bmSync:public NetThread
{
	Q_OBJECT;
public:
	QSemaphore *semaphore;
	QSqlDatabase *db;
	

	QString username;
	QString password;

	volatile bool needwatchchild;	

	//DoNetThread *doTestNetThread;
	DoNetThread *doNetThread;
	bmMerge *mgthread;	
	uint bmSyncMode;
	uint diggid;	
	
public:
	bmSync(QObject * parent = 0,QSettings* s=0,QSqlDatabase *db=0,QSemaphore* p=NULL,int m=SYNC_DO_BOOKMARK);
	~bmSync(){};
	void setUsername(const QString& s){username = s;}
	void setPassword(const QString& s){password = s;}
	void run();
	void setDiggId(uint id){diggid = id;}	
public slots: 
	void bmxmlGetFinished(int status);
	void diggxmlGetFinished(int);
	void doTestNetFinished();
//	void testAccountFinished(bool error);
	void mergeDone();
	void testNetFinished(int status);
	void terminateThread();
	void monitorTimeout();
	virtual void cleanObjects();
	virtual void sendUpdateStatusNotify(int status){		
		statusCode = status;
		if(doWhat==SYNC_DO_BOOKMARK&&bmSyncMode==SYN_MODE_SILENCE) 
			return;	
		TD(DEBUG_LEVEL_NORMAL,status<<tz::getstatusstring(status));
		emit updateStatusNotify(status);
	}
signals:
	void bmSyncFinishedStatusNotify(int status);
	void readDateProgressNotify(int done, int total);
	void testAccountFinishedNotify(int status);
};

#define LOCAL_EXIST_OFFSET  2
#define LASTUPDATE_EXIST_OFFSET 1
#define SERVER_EXIST_OFFSET 0
#endif
