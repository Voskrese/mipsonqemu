#include <appupdater.h>
#include <bmapi.h>
#include <config.h>
/*
void appUpdater::sendUpdateStatusNotify(int flag,int type,int icon)
{
	if(mode!=UPDATE_DLG_MODE) 
		return;
	emit updateStatusNotify(flag,type,icon);	
}
*/
appUpdater::~appUpdater(){

}

void appUpdater::testNetFinished()
{
	switch(GET_RUN_PARAMETER(RUN_PARAMETER_TESTNET_RESULT))
	{
	case TEST_NET_ERROR_SERVER:
		//sendUpdateStatusNotify(UPDATESTATUS_FLAG_RETRY,UPDATE_NET_ERROR,UPDATE_STATUS_ICON_FAILED);
		sendUpdateStatusNotify(UPDATE_NET_ERROR);
		error = 1;
		quit();
		break;
	case TEST_NET_REFUSE:
		sendUpdateStatusNotify(UPDATE_SERVER_REFUSE);
		error = 1;
		quit();
		break;
	case TEST_NET_ERROR_PROXY:
		sendUpdateStatusNotify(UPDATE_NET_ERROR_PROXY);
		error = 1;
		quit();
		break;
	case TEST_NET_ERROR_PROXY_AUTH:
		sendUpdateStatusNotify(UPDATE_NET_ERROR_PROXY_AUTH);
		error = 1;
		quit();
		break;
	case TEST_NET_SUCCESS:
		{
			donetThread = new DoNetThread(this,settings,DOWHAT_TEST_SERVER_VERSION);
			donetThread->moveToThread(this);
			donetThread->start(QThread::IdlePriority);				
		}
		break;
	}	
}
void appUpdater::testVersionFinished()
{
	switch(GET_RUN_PARAMETER(RUN_PARAMETER_TESTNET_VERSION))
	{
		case 0:
			quit();
			break;
		case 1:
			downloadFileFromServer(UPDATE_SERVER_URL,UPDATE_MODE_GET_INI,"");
			break;
	}
}
void appUpdater::terminateThread()
{	
	QDEBUG_LINE;
	STOP_TIMER(monitorTimer);
	if(THREAD_IS_RUNNING(donetThread))
		donetThread->setTerminateFlag(1);
	if(THREAD_IS_RUNNING(fh))
		fh->setTerminateFlag(1);
}
void appUpdater::monitorTimeout()
{
	THREAD_MONITOR_POINT;
	STOP_TIMER(monitorTimer);
	 if(THREAD_IS_FINISHED(donetThread))
	 {
	 	if(donetThread->doWhat==DOWHAT_TEST_SERVER_NET){
			DELETE_OBJECT(donetThread);
	 		testNetFinished();
	 	}else if(donetThread->doWhat==DOWHAT_TEST_SERVER_VERSION){
	 		QDEBUG_LINE;
	 		DELETE_OBJECT(donetThread);
	 		testVersionFinished();
	 	}
	 }

	 if(THREAD_IS_FINISHED(fh))
	 {
		if(fh->doWhat==UPDATE_MODE_GET_INI)
			{
				int err = fh->errCode;
				fh->wait();
				DELETE_OBJECT(fh);
				getIniDone(err);
			}
	 }
	if(!needwatchchild&&terminateFlag)
	{
		needwatchchild = true;
		terminateThread();
	}
	monitorTimer->start(tz::getParameterMib(SYS_MONITORTIMEOUT));	
}
void appUpdater::cleanObjects(){
	DELETE_OBJECT(donetThread);
	if(fh)
		fh->wait();
	DELETE_OBJECT(fh);
	DELETE_OBJECT(localSettings);
	DELETE_OBJECT(serverSettings);
	NetThread::cleanObjects();
}

void appUpdater::run()
{
	NetThread::run();
	if(dlgmode == UPDATE_DLG_MODE )
		connect(this, SIGNAL(updateStatusNotify(int)), this->parent(), SLOT(updateStatus(int)));
	
	donetThread = new DoNetThread(this,settings,DOWHAT_TEST_SERVER_NET,0);
	donetThread->setUrl(TEST_NET_URL);
	donetThread->moveToThread(this);
	donetThread->start(QThread::IdlePriority);	
	exec();
	cleanObjects();
}

/*
m:0--local to server ,1--server to local
flag:0--download file 1---checkfile
*/
int appUpdater::mergeSettings(QSettings* srcSettings,QSettings* dstSetting,int m)
{
	//merge local with server
	int count = srcSettings->beginReadArray(UPDATE_PORTABLE_KEYWORD);
	for (int i = 0; i < count; i++)
	{
		if(terminateFlag||error)
			break;
		srcSettings->setArrayIndex(i);
		QString f=srcSettings->value("name").toString();
		QString md5=srcSettings->value("md5","").toString(); 
		int  flag =tz::checkToSetting(dstSetting,f,md5);
		switch(flag)
		{
		case -1://no found
		case 1://newer
			if(m==SETTING_MERGE_SERVERTOLOCAL)
				md5=srcSettings->value("md5","").toString(); //reupdate md5,just md5 from server is valid
			if(((m==SETTING_MERGE_SERVERTOLOCAL)&&(flag==-1))||((m==SETTING_MERGE_LOCALTOSERVER)&&(flag==1)))
			{
				if(
					(!QFile::exists(QString(UPDATE_PORTABLE_DIRECTORY).append(f))||
					(md5!=tz::fileMd5(QString(UPDATE_PORTABLE_DIRECTORY).append(f))))&&
					(!QFile::exists(f)||(md5!=tz::fileMd5(f)))
				  )
				{
					needed=1;
					downloadFileFromServer(f,UPDATE_MODE_GET_FILE,md5);	

				}
			}
			break;
		default:
			break;
		}
	}
end:
	srcSettings->endArray();
	return needed;
}
void  appUpdater::checkSilentUpdateApp()
{
	if(QFile::exists(QString(UPDATE_PORTABLE_DIRECTORY).append(APP_SILENT_UPDATE_NAME)))		
	{
		QFile::copy(QString(UPDATE_PORTABLE_DIRECTORY).append(APP_SILENT_UPDATE_NAME),APP_SILENT_UPDATE_NAME);
		//QFile::remove(QString(UPDATE_PORTABLE_DIRECTORY).append(APP_SILENT_UPDATE_NAME));
	}
}
void appUpdater::getIniDone(int err)
{
	THREAD_MONITOR_POINT;
	if(terminateFlag||error)	goto end;
	switch(dlgmode){
		case UPDATE_SILENT_MODE:				
			if(!err){
				//merge local with server
				localSettings = new QSettings(UPDATE_FILE_NAME, QSettings::IniFormat, NULL);
				serverSettings = new QSettings(QString(UPDATE_PORTABLE_DIRECTORY).append(UPDATE_FILE_NAME), QSettings::IniFormat, NULL);
				mergeSettings(localSettings,serverSettings,SETTING_MERGE_LOCALTOSERVER);
				mergeSettings(serverSettings,localSettings,SETTING_MERGE_SERVERTOLOCAL);
				if(terminateFlag)	goto end;
				if(!error&&needed) 
				{
					checkSilentUpdateApp();
					tz::registerInt(REGISTER_SET_MODE,APP_HKEY_PATH,APP_HEKY_UPDATE_ITEM,1);
					//write update.ini
					/*
					int count = serverSettings->beginReadArray(UPDATE_PORTABLE_KEYWORD);
					localSettings->beginWriteArray(UPDATE_PORTABLE_KEYWORD);
					for (int i = 0; i < count; i++)
							{
								serverSettings->setArrayIndex(i);								
								localSettings->setArrayIndex(i);
								localSettings->setValue("name",serverSettings->value("name").toString());
								localSettings->setValue("md5",serverSettings->value("md5").toString()); 
							}
					serverSettings->endArray();
					localSettings->endArray();
					localSettings->sync();
					*/
				}
			}
			break;
		case UPDATE_DLG_MODE:
			if(!err){
				serverSettings = new QSettings(QString(UPDATE_SETUP_DIRECTORY).append(UPDATE_FILE_NAME), QSettings::IniFormat, NULL);
				QString f = serverSettings->value("setup/name", "").toString();
				QString servermd5 = serverSettings->value("setup/md5", "").toString();
				if(f.isEmpty()||servermd5.isEmpty())
				{
					//get wrong content form server
					sendUpdateStatusNotify(UPDATE_FAILED);
					error = 1;
					goto end;
				}
				if (QFile::exists(UPDATE_FILE_NAME))
				{
					localSettings = new QSettings(UPDATE_FILE_NAME, QSettings::IniFormat, NULL);
					QString localmd5 = serverSettings->value("setup/md5", "").toString();
					if( servermd5 == localmd5)
					{
						sendUpdateStatusNotify(UPDATE_NO_NEED);					
						goto end;
					}
				}
				//start download setup.exe
				if(!f.isEmpty()&&!servermd5.isEmpty())
				{
					needed = 1;
					downloadFileFromServer(f,UPDATE_MODE_GET_FILE,servermd5);
				}
				if(terminateFlag||error)
					goto end;
				if(error){
					sendUpdateStatusNotify(UPDATE_FAILED);
				}else if(needed) 
				{
					sendUpdateStatusNotify(UPDATE_SUCCESSFUL);
					//write update.ini
					if(!localSettings)
						localSettings = new QSettings(UPDATE_FILE_NAME, QSettings::IniFormat, NULL);
					localSettings->setValue("setup/name",f);
					localSettings->setValue("setup/md5",servermd5);
					localSettings->sync();
				}		

			}else{
				sendUpdateStatusNotify(UPDATE_FAILED);
			}
			break;
		default:
			break;
	}

	//msleep(500);
	//find the "lanuchy"

	//HWND	 hWnd	= ::FindWindow(NULL, (LPCWSTR) QString("debug").utf16());
	//qDebug("hWnd=0x%08x\n",hWnd);
	//::SendMessage(hWnd, WM_CLOSE, 0, 0);
//	QDEBUG_LINE;
end:
	quit();

}
void appUpdater::downloadFileFromServer(QString pathname,int m,QString md5)
{
	THREAD_MONITOR_POINT;
	if(terminateFlag||error)
		return;
	qDebug()<<"download......"<<pathname<<"md5:"<<md5;
	DELETE_OBJECT(fh);
	fh=new GetFileHttp(NULL,settings,m,md5);

	if(dlgmode==UPDATE_DLG_MODE) {
		connect(fh, SIGNAL(updateStatusNotify(int)), this->parent(), SLOT(updateStatus(int)));
		connect(this, SIGNAL(updateStatusNotify(int)), this->parent(), SLOT(updateStatus(int)));
	}
/*
#ifdef CONFIG_SERVER_IP_SETTING
	SET_HOST_IP(settings,fh);
#else
	fh->setHost(UPDATE_SERVER_HOST);
#endif
*/
//	SET_HOST_IP(settings,fh,&url,header);
	fh->setDestdir(UPDATE_DIRECTORY);
	fh->setServerBranch("/download");
	//htttp://www.tanzhi.com/download/setup/tanzhi.exe
	//htttp://www.tanzhi.com/download/portable/tanzhi.exe
	switch(dlgmode)
	{
	case UPDATE_DLG_MODE:
		fh->setUrl(QString("setup/").append(pathname));
		break;
	case UPDATE_SILENT_MODE:
		fh->setUrl(QString("portable/").append(pathname));
		break;
	}

	fh->start(QThread::IdlePriority);
	 if(fh&&(fh->doWhat==UPDATE_MODE_GET_FILE))
		fh->wait();
	 error = fh->errCode;
	 qDebug()<<pathname<<" error code:"<<error;
}


