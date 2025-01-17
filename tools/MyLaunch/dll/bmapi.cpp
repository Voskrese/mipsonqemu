#include <bmapi.h>


#define SystemBasicInformation 0 
#define SystemPerformanceInformation 2 
#define SystemTimeInformation 3

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct 
{ 
	DWORD dwUnknown1; 
	ULONG uKeMaximumIncrement; 
	ULONG uPageSize; 
	ULONG uMmNumberOfPhysicalPages; 
	ULONG uMmLowestPhysicalPage; 
	ULONG uMmHighestPhysicalPage; 
	ULONG uAllocationGranularity; 
	PVOID pLowestUserAddress; 
	PVOID pMmHighestUserAddress; 
	ULONG uKeActiveProcessors; 
	BYTE bKeNumberProcessors; 
	BYTE bUnknown2; 
	WORD wUnknown3; 
} SYSTEM_BASIC_INFORMATION;

typedef struct 
{ 
	LARGE_INTEGER liIdleTime; 
	DWORD dwSpare[76]; 
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct 
{ 
	LARGE_INTEGER liKeBootTime; 
	LARGE_INTEGER liKeSystemTime; 
	LARGE_INTEGER liExpTimeZoneBias; 
	ULONG uCurrentTimeZoneId; 
	DWORD dwReserved; 
} SYSTEM_TIME_INFORMATION;

typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);

PROCNTQSI NtQuerySystemInformation;

//uint gMaxGroupId=0;
QString gUpdatetime;
//bool gPostError=0;
//uint gPostResponse=0;
//uint gBmId=0;
int language=DEFAULT_LANGUAGE;

/*
	static string
*/
//static QString iePath;
//static QString localbmdatfullpath;
//static QString localDbFullpath;
//static QString localIniFullpath;
static QString localfullpath[LOCAL_FULLPATH_MAX];

char* gLanguageList[]={"ch","en"};

struct{
	uint status;
	char* statusstr;
}statusStrings[]={
	{LOADING_MIN,"LOADING_MIN"},
	{HTTP_UNCONNECTED,"HTTP_UNCONNECTED"},
	{HTTP_HOSTLOOKUP,"HTTP_HOSTLOOKUP"},
	{HTTP_CONNECTING,"HTTP_CONNECTING"},
	{HTTP_SENDING,"HTTP_SENDING"},
	{HTTP_READING,"HTTP_READING"},
	{HTTP_CONNECTED,"HTTP_CONNECTED"},
	{HTTP_CLOSING,"HTTP_CLOSING"},
	{HTTP_TIMEOUT,"HTTP_TIMEOUT"},
	{UPDATE_PROCESSING,"UPDATE_PROCESSING"},
	{TRY_CONNECT_SERVER,"TRY_CONNECT_SERVER"},
	{BM_SYNC_START,"BM_SYNC_START"},
	{BM_TESTACCOUNT_START,"BM_TESTACCOUNT_START"},
	{BM_TESTVERSION_START,"BM_TESTVERSION_START"},
	{LOADING_MAX,"LOADING_MAX"},
	{SUCCESS_MIN,"SUCCESS_MIN"},
	{TEST_NET_SUCCESS,"TEST_NET_SUCCESS"},
	{TEST_VERSION_UNNEED,"TEST_VERSION_UNNEED"},
	{DOWHAT_GET_FILE_SUCCESS,"DOWHAT_GET_FILE_SUCCESS"},
	{BM_MERGE_START,"BM_MERGE_START"},	
	{HTTP_TEST_ACCOUNT_SUCCESS,"HTTP_TEST_ACCOUNT_SUCCESS"},
	{UPDATE_SUCCESSFUL,"UPDATE_SUCCESSFUL"},
	{BM_SYNC_SUCCESS_NO_MODIFY,"BM_SYNC_SUCCESS_NO_MODIFY"},
	{BM_SYNC_SUCCESS_WITH_MODIFY,"BM_SYNC_SUCCESS_WITH_MODIFY"},
	{UPDATE_NO_NEED,"UPDATE_NO_NEED"},
	{TEST_VERSION_SUCCESS,"TEST_VERSION_SUCCESS"},
	{TEST_DIGGXML_SUCCESS,"TEST_DIGGXML_SUCCESS"},
	{TEST_DIGGXML_UNNEED,"TEST_DIGGXML_UNNEED"},
	{SUCCESS_MAX,"SUCCESS_MAX"},
	{REFUSE_MIN,"REFUSE_MIN"},
	{TEST_NET_REFUSE,"TEST_NET_REFUSE"},
	{UPDATE_SERVER_REFUSE,"UPDATE_SERVER_REFUSE"},
	{BM_SYNC_FAIL_SERVER_REFUSE,"BM_SYNC_FAIL_SERVER_REFUSE"},
	{REFUSE_MAX,"REFUSE_MAX"},
	{FAIL_MIN,"FAIL_MIN"},
	{TEST_NET_ERROR_PROXY_AUTH,"TEST_NET_ERROR_PROXY_AUTH"},
	{TEST_NET_ERROR_SERVER,"TEST_NET_ERROR_SERVER"},
	{HTTP_TEST_ACCOUNT_FAIL,"HTTP_TEST_ACCOUNT_FAIL"},
	{DOWHAT_GET_FILE_FAIL,"DOWHAT_GET_FILE_FAIL"},
	{HTTP_NEED_RETRY,"HTTP_NEED_RETRY"},
	{UPDATE_FAILED,"UPDATE_FAILED"},
	{BM_SYNC_FAIL_LOGIN,"BM_SYNC_FAIL_LOGIN"},
	{BM_SYNC_FAIL_POST_HTTP,"BM_SYNC_FAIL_POST_HTTP"},
	{BM_SYNC_FAIL_SERVER_NET_ERROR,"BM_SYNC_FAIL_SERVER_NET_ERROR"},
	{BM_SYNC_FAIL_SERVER_BMXML_FAIL,"BM_SYNC_FAIL_SERVER_BMXML_FAIL"},
	{BM_SYNC_FAIL_BMXML_TIMEOUT,"BM_SYNC_FAIL_BMXML_TIMEOUT"},
	{BM_SYNC_FAIL_MERGE_ERROR,"BM_SYNC_FAIL_MERGE_ERROR"},
	{BM_SYNC_FAIL_PROXY_ERROR,"BM_SYNC_FAIL_PROXY_ERROR"},
	{BM_SYNC_FAIL_PROXY_AUTH_ERROR,"BM_SYNC_FAIL_PROXY_AUTH_ERROR"},
	{BM_SYNC_FAIL_SERVER_TESTACCOUNT_FAIL,"BM_SYNC_FAIL_SERVER_TESTACCOUNT_FAIL"},
	{BM_SYNC_FAIL_SERVER_LOGIN,"BM_SYNC_FAIL_SERVER_LOGIN"},
	{BM_SYNC_FAIL_DOWNLOCAL_WRITE_FILE,"BM_SYNC_FAIL_DOWNLOCAL_WRITE_FILE"},
	{BM_SYNC_FAIL_DOWNLOCAL_DELETE_FILE,"BM_SYNC_FAIL_DOWNLOCAL_DELETE_FILE"},
	{BM_SYNC_FAIL_DOWNLOCAL_DB_LOCK,"BM_SYNC_FAIL_DOWNLOCAL_DB_LOCK"},
	{BM_SYNC_FAIL_DOWNLOCAL_DB_QUERY,"BM_SYNC_FAIL_DOWNLOCAL_DB_QUERY"},
	{BM_SYNC_FAIL_EXCEED_TOTAL_NUM,"BM_SYNC_FAIL_EXCEED_TOTAL_NUM"},
	{BM_SYNC_FAIL_EXCEED_DIR_COUNT,"BM_SYNC_FAIL_EXCEED_DIR_COUNT"},
	{BM_SYNC_FAIL_EXCEED_LEVEL,"BM_SYNC_FAIL_EXCEED_LEVEL"},
	{BM_SYNC_FAIL_GET_XML_FROM_SERVER,"BM_SYNC_FAIL_GET_XML_FROM_SERVER"},
	{BM_SYNC_FAIL_READ_XML_FROM_SERVER,"BM_SYNC_FAIL_READ_XML_FROM_SERVER"},
	{BM_SYNC_FAIL_READ_XML_LOCAL,"BM_SYNC_FAIL_READ_XML_LOCAL"},
	{BM_SYNC_FAIL_REMOVE_XML_LOCAL,"BM_SYNC_FAIL_REMOVE_XML_LOCAL"},
	{FAIL_MAX,"FAIL_MAX"},
	{SYNC_STATUS_MAX,"SYNC_STATUS_MAX"},
};


//static int testnetresult = 0;
//static int netProxyEnable = 0;
static int runparameters[RUN_PARAMETER_END]={0,0,0,0,0,0,0};
static QNetworkProxy *netproxy = NULL;
QList<struct dbtableinfo*> dbtableInfolist;

struct dbtableinfo dbtableInfo[]={
	{COME_FROM_SHORTCUT,QString(DB_TABLE_SUFFIX"_shortcut"),COME_FROM_SHORTCUT},
	{COME_FROM_PREDEFINE,QString(DB_TABLE_SUFFIX"_predefine"),COME_FROM_PREDEFINE},
	{COME_FROM_COMMAND,QString(DB_TABLE_SUFFIX"_command"),COME_FROM_COMMAND},
	{COME_FROM_PROGRAM,QString(DB_TABLE_SUFFIX"_program"),COME_FROM_PROGRAM},
	{COME_FROM_LEARNPROCESS,QString(DB_TABLE_SUFFIX"_learnprocess"),COME_FROM_LEARNPROCESS},	
	{COME_FROM_MYBOOKMARK,QString(DB_TABLE_SUFFIX"_mybookmark"),COME_FROM_MYBOOKMARK},	
	{COME_FROM_BROWSER,QString(DB_TABLE_SUFFIX"_browser"),COME_FROM_BROWSER},
	{0,QString(),0}
};

/*
$_SGLOBAL['browser']=Array
	(
	0 => Array
		(
		'browserid' => '0',
		'browsername' => 'network',
		'maxlev' => 5,
		'maxchild' => 128,
		'titlelen' => 512,
		'dirlen' => 256,
		'urllen' => 512,
		'taglen' => 128,
		'deslen' => 512,
		'speicalchar' => 1
		),
	1 => Array
		(
		'browserid' => 1,
		'browsername' => 'ie',
		'maxlev' => 5,
		'maxchild' => 128,
		'titlelen' => 210,
		'dirlen' => 202,
		'urllen' => 512,
		'taglen' => 128,
		'deslen' => 512,
		'speicalchar' => '0'
		),
	2 => Array
		(
		'browserid' => 2,
		'browsername' => 'firefox',
		'maxlev' => 6,
		'maxchild' => 128,
		'titlelen' => 512,
		'dirlen' => 256,
		'urllen' => 512,
		'taglen' => 128,
		'deslen' => 512,
		'speicalchar' => 1
		),
	3 => Array
		(
		'browserid' => 3,
		'browsername' => 'opera',
		'maxlev' => 5,
		'maxchild' => 128,
		'titlelen' => 512,
		'dirlen' => 256,
		'urllen' => 512,
		'taglen' => 128,
		'deslen' => 512,
		'speicalchar' => 1
		)
	)

*/
struct browserinfo browserInfo[]={
	{QString("network"),QString("") ,true, true, false,false,false, BROWSE_TYPE_NETBOOKMARK,5,128,512,256,512,128,512,1},
	{QString("Ie"),QString(""), true, true, false,false,false, BROWSE_TYPE_IE,5,128,210,202,512,128,512,0},
	{QString("Firefox"),QString("") , false, false , false,false,false, BROWSE_TYPE_FIREFOX,6,128,512,256,512,128,512,1},
	{QString("Opera") , QString("") ,false , false , false,false,false, BROWSE_TYPE_OPERA,5,128,512,256,512,128,512,1},
	{QString(""),QString("") ,false,false, false,false,false,0}
};
uint  getBrowserInfoParameter(int browserid,int type)
{
	uint ret = 0;
	switch(type){
		case BROWSER_INFO_ID:
			ret = browserInfo[browserid].id;
			break;
		case BROWSER_INFO_MAXLEV:
			ret = browserInfo[browserid].maxlev;
			break;
		case BROWSER_INFO_MAXCHILD:
			ret = browserInfo[browserid].maxchild;
			break;
		case BROWSER_INFO_TITLELEN:
			ret = browserInfo[browserid].titlelen;
			break;
		case BROWSER_INFO_DIRLEN:
			ret = browserInfo[browserid].dirlen;
			break;
		case BROWSER_INFO_URLLEN:
			ret = browserInfo[browserid].urllen;
			break;
		case BROWSER_INFO_TAGLEN:
			ret = browserInfo[browserid].taglen;
			break;
		case BROWSER_INFO_DESLEN:
			ret = browserInfo[browserid].deslen;
			break;
		case BROWSER_INFO_SPEICALCHAR:
			ret = browserInfo[browserid].speicalchar;
			break;
	}
	return ret;
}
void setBrowserInfoOpFlag(uint id,enum BROWSERINFO_OP type)
{
	int i = 0;
	while(!browserInfo[i].name.isEmpty())
	{
		if( browserInfo[i].id == id )
		{
			switch(type){
				case BROWSERINFO_OP_LASTUPDATE:
					browserInfo[i].lastupdate= true;
					break;
				case BROWSERINFO_OP_FROMSERVER:
					browserInfo[i].fromserver= true;
					break;
				case BROWSERINFO_OP_LOCAL:
					browserInfo[i].local= true;
					break;
			}					
			return;
		}
		i++;
	}
	return ;
}
void clearBrowserInfoOpFlag(uint id)
{
	int i = 0;
	while(!browserInfo[i].name.isEmpty())
	{
		if( browserInfo[i].id == id )
		{
			browserInfo[i].lastupdate= false;
			browserInfo[i].fromserver= false;
			browserInfo[i].local= false;
			return;
		}
		i++;
	}
	return ;
}
void setBrowserFullpath(QSettings *s,int type,QString& fullpath){
	switch(type){
		case BROWSE_TYPE_NETBOOKMARK:
				fullpath = s->value("netbookmarkbrowser","").toString();
				if(fullpath.isEmpty())
					fullpath = tz::getUserFullpath(NULL,LOCAL_FULLPATH_DEFBROWSER);
			break;
		case BROWSE_TYPE_IE:
			{
				fullpath = s->value("adv/iepath","").toString();
				if(fullpath.isEmpty())
					fullpath = tz::getUserFullpath(NULL,LOCAL_FULLPATH_IE);
			}
			break;
		case BROWSE_TYPE_FIREFOX:
			{
				fullpath = s->value("adv/firefoxpath","").toString();
				if(fullpath.isEmpty())
					fullpath=tz::getUserFullpath(NULL,LOCAL_FULLPATH_FIREFOX);
			}
			break;
		case BROWSE_TYPE_OPERA:
			break;
		default:
			break;
	}
}


bool getBrowserEnable(uint id)
{
	int i = 0;
	while(!browserInfo[i].name.isEmpty())
	{
		if( browserInfo[i].id == id )
			return browserInfo[i].enable;
		i++;
	}
	return false;
}
void setBrowserEnable(QSettings *s)
{
	if(!s) return;
	int i = 0;
	while(!browserInfo[i].name.isEmpty())
	{
		browserInfo[i].enable = s->value(QString("adv/ckSupport%1").arg(browserInfo[i].name),browserInfo[i].defenable).toBool();
		if(browserInfo[i].enable){
			setBrowserFullpath(s,browserInfo[i].id,browserInfo[i].fullpath);
			qDebug()<<browserInfo[i].id<<browserInfo[i].fullpath;
		}
		i++;
	}
}

void getBrowserFullpath(int type,QString& fullpath){
	fullpath = browserInfo[type].fullpath;
	return;
}

struct handleItemInfo handleiteminfo;

void setHandleItemInfo(QString& n,QString& f,uint i,uint a)
{
	handleiteminfo.name = n;
	handleiteminfo.fullpath = f;
	handleiteminfo.browerid = i;
	handleiteminfo.action = a;
}

struct handleItemInfo* getHandleItemInfo()
{
	return &handleiteminfo;
}
#ifdef CONFIG_ACTION_LIST
QList <ACTION_LIST> actionlist;
int addToActionList( struct ACTION_LIST& item){
	for (int i = 0; i < actionlist.size(); ++i) {
	   if((actionlist[i].fullpath!=item.fullpath))
	   	continue;
	     if((actionlist[i].name!=item.name))
	   	continue;	   	
	   if((actionlist[i].action!=item.action))
	   	continue;
	   if((actionlist[i].id.groupid!=item.id.groupid)){
	   	if((item.action == ACTION_LIST_BOOKMARK_SYNC)||(item.action == ACTION_LIST_TEST_ACCOUNT))
	   		{
	   			if((item.id.groupid==SYN_MODE_NOSILENCE)||(actionlist[i].id.groupid==SYN_MODE_NOSILENCE)){
					actionlist[i].id.groupid=SYN_MODE_NOSILENCE;
					return 0;
	   			}
	   		}
	   }
	   
	   	return 0;
 	} 
	actionlist.push_back(item);
	return 0;
}
int getFromActionList(struct ACTION_LIST& item){
	if(actionlist.empty())
		return 0;
	item=actionlist.takeFirst();
	return 1;
}
#endif

/*


void setPostResponse(uint  type)
{
	gPostResponse=type;
}
uint getPostResponse()
{
	return gPostResponse;
}



void setPostError(bool err)
{
	gPostError=err;
}
bool getPostError()
{
	return gPostError;
}

void setMaxGroupId(uint id)
{
	gMaxGroupId=id;
}
uint getMaxGroupId()
{
	return gMaxGroupId;
}


void setBmId(uint bmid)
{
	gBmId=bmid;
}
uint getBmId()
{
	return gBmId;
}
*/
void setUpdatetime(QString time)
{
	gUpdatetime=time;
}
void getUpdatetime(QString& time)
{
	time=gUpdatetime;
}


#if 0
extern QDateTime gNowUpdateTime;;
int setDirectoryTimeIncludeAllFiles(QString path)
{
	QDir qd(path);
	QString dir = qd.absolutePath();
	QStringList dirs = qd.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < dirs.count(); ++i)
	{
		QString cur = dirs[i];
		if (cur.contains(".lnk"))
			continue;
		setDirectoryTimeIncludeAllFiles(dir + "/" + dirs[i]);
		setFileTime(dir + "/" + dirs[i], gNowUpdateTime.toString(TIME_FORMAT), NULL, NULL, NAME_IS_FILE);
	}

	QStringList files = qd.entryList(QStringList("*.url"), QDir::Files, QDir::Unsorted);
	for (int i = 0; i < files.count(); ++i)
	{
		setFileTime(path + "/" + files[i], gNowUpdateTime.toString(TIME_FORMAT), NULL, NULL, NAME_IS_FILE);
	}
	return 1;
}

int getFileTime(QString filename, QString * createTime, QString * lastAccessTime, QString * lastWriteTime, int flag)
{
	HANDLE hfile;
	FILETIME lpCreationTime, lpCreationTime1;	// creation time
	FILETIME lpLastAccessTime;	// last access time
	FILETIME lpLastWriteTime;	// last write time
	SYSTEMTIME lpSysCreationTime;
	SYSTEMTIME lpSysLastAccessTime;
	SYSTEMTIME lpSysLastWriteTime;

	hfile = CreateFile((LPCWSTR) filename.utf16(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, flag ? FILE_FLAG_BACKUP_SEMANTICS : FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		// logToFile("Open %s failed!\nErrorCode:%d\n", qPrintable(filename), GetLastError());
		return NULL;
	}
	if (hfile)
	{
		GetFileTime((HANDLE) hfile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
		if (!FileTimeToLocalFileTime(&lpCreationTime, &lpCreationTime1))
			return NULL;
		FileTimeToSystemTime(&lpCreationTime1, &lpSysCreationTime);
		createTime->sprintf("%04d-%02d-%02d %02d:%02d:%02d", lpSysCreationTime.wYear, lpSysCreationTime.wMonth, lpSysCreationTime.wDay, lpSysCreationTime.wHour, lpSysCreationTime.wMinute, lpSysCreationTime.wSecond);
		if (!FileTimeToLocalFileTime(&lpLastAccessTime, &lpCreationTime1))
			return NULL;
		FileTimeToSystemTime(&lpCreationTime1, &lpSysLastAccessTime);
		lastAccessTime->sprintf("%04d-%02d-%02d %02d:%02d:%02d", lpSysLastAccessTime.wYear, lpSysLastAccessTime.wMonth, lpSysLastAccessTime.wDay, lpSysLastAccessTime.wHour, lpSysLastAccessTime.wMinute, lpSysLastAccessTime.wSecond);
		if (!FileTimeToLocalFileTime(&lpLastWriteTime, &lpCreationTime1))
			return NULL;
		FileTimeToSystemTime(&lpCreationTime1, &lpSysLastWriteTime);
		lastWriteTime->sprintf("%04d-%02d-%02d %02d:%02d:%02d", lpSysLastWriteTime.wYear, lpSysLastWriteTime.wMonth, lpSysLastWriteTime.wDay, lpSysLastWriteTime.wHour, lpSysLastWriteTime.wMinute, lpSysLastWriteTime.wSecond);
		CloseHandle(hfile);
		return NULL;
	}
	return NULL;
}
int setFileTime(QString filename, QString createTime, QString * lastAccessTime, QString * lastWriteTime, int flag)
{
#ifdef CONFIG_LOG_ENABLE
	//logToFile("%s file=%s time=%s",__FUNCTION__,qPrintable(filename),qPrintable(createTime));
#endif
	HANDLE hfile;
	// FILETIME lpCreationTime,lpCreationTime1;   // creation time
	// FILETIME lpLastAccessTime;  // last access time
	FILETIME LastWriteTime, LastWriteTime1;	// last write time
	// SYSTEMTIME  lpSysCreationTime;
	// SYSTEMTIME  lpSysLastAccessTime;
	SYSTEMTIME SysLastWriteTime;
	hfile = CreateFile((LPCWSTR) filename.utf16(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, flag ? FILE_FLAG_BACKUP_SEMANTICS : FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		//logToFile("Open %s failed!\nErrorCode:%d\n", qPrintable(filename), GetLastError());
		return NULL;
	}
	int year, month, day, hour, minute, second;
	if (hfile)
	{
		memset(&SysLastWriteTime, 0, sizeof(SysLastWriteTime));
		sscanf(createTime.toUtf8(), "%u-%u-%u %u:%u:%u", &year, &month, &day, &hour, &minute, &second);
		SysLastWriteTime.wYear = year;
		SysLastWriteTime.wMonth = month;
		SysLastWriteTime.wDay = day;
		SysLastWriteTime.wHour = hour;
		SysLastWriteTime.wMinute = minute;
		SysLastWriteTime.wSecond = second;
#ifdef CONFIG_LOG_ENABLE
		//      logToFile("%s time=%u-%02u-%02u %02u:%02u:%02u",__FUNCTION__,SysLastWriteTime.wYear,SysLastWriteTime.wMonth,SysLastWriteTime.wDay,
		//              SysLastWriteTime.wHour, SysLastWriteTime.wMinute,SysLastWriteTime.wSecond);
#endif
#if 0
		//GetSystemTime(&lpSysCreationTime);              
		SystemTimeToFileTime(&lpSysCreationTime, &lpLastWriteTime1);
#else
		SystemTimeToFileTime(&SysLastWriteTime, &LastWriteTime);	// converts to file time format
		LocalFileTimeToFileTime(&LastWriteTime, &LastWriteTime1);
#endif
		if (!SetFileTime(hfile, &LastWriteTime1, (LPFILETIME) NULL, &LastWriteTime1))
		{
			//logToFile("setfiletime error %d", GetLastError());
		}

		CloseHandle(hfile);
		return NULL;
	}
	return NULL;
}
#endif



void runProgram(QString path, QString args) {
#ifdef Q_WS_WIN
	SHELLEXECUTEINFO ShExecInfo;
	//qDebug("runProgram path=%s args=%s",qPrintable(path),qPrintable(args));
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_FLAG_NO_UI;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = (LPCTSTR) (path).utf16();
	if (args != "") {
		ShExecInfo.lpParameters = (LPCTSTR) args.utf16();
	} else {
		ShExecInfo.lpParameters = NULL;
	}
	QDir dir(path);
	QFileInfo info(path);
	if (!info.isDir() && info.isFile())
		dir.cdUp();
	ShExecInfo.lpDirectory = (LPCTSTR)QDir::toNativeSeparators(dir.absolutePath()).utf16();
	ShExecInfo.nShow = SW_NORMAL;
	ShExecInfo.hInstApp = NULL;

	ShellExecuteEx(&ShExecInfo);	
#endif

#ifdef Q_WS_MAC

#endif

#ifdef Q_WS_X11

#endif

}
BOOL GetShellDir(int iType, QString & szPath)
{
	QString tmpp = "shell32.dll";
	HINSTANCE hInst =::LoadLibrary((LPCTSTR) tmpp.utf16());
	if (NULL == hInst)
	{
		return FALSE;
	}


	HRESULT(__stdcall * pfnSHGetFolderPath) (HWND, int, HANDLE, DWORD, LPWSTR);


	pfnSHGetFolderPath = reinterpret_cast < HRESULT(__stdcall *) (HWND, int, HANDLE, DWORD, LPWSTR) >(GetProcAddress(hInst, "SHGetFolderPathW"));

	if (NULL == pfnSHGetFolderPath)
	{
		FreeLibrary(hInst);	// <-- here
		return FALSE;
	}

	TCHAR tmp[_MAX_PATH];
	pfnSHGetFolderPath(NULL, iType, NULL, 0, tmp);
	szPath = QString::fromUtf16((const ushort *) tmp);
	FreeLibrary(hInst);	// <-- and here
	return TRUE;
}
QString tz::GetShortcutTarget(const QString& LinkFileName)
{
	HRESULT hres;
	TCHAR arg[1024]={0};

	QString Link, Temp = LinkFileName;
	HRESULT (__stdcall * CoInitialize)(LPVOID);
	void  (__stdcall * CoUninitialize)(void );
	CoInitialize = reinterpret_cast < HRESULT(__stdcall *) (LPVOID ) >(GetProcAddress(GetModuleHandle(TEXT("ole32")),"CoInitialize"));
	CoUninitialize = reinterpret_cast < void (__stdcall *) (void ) >(GetProcAddress(GetModuleHandle(TEXT("ole32")),"CoUninitialize"));
	if(!CoInitialize||!CoUninitialize)
		return "";
	if(SUCCEEDED(CoInitialize(NULL))){
		HRESULT (__stdcall * CoCreateInstance)( REFCLSID ,   LPUNKNOWN ,   DWORD , REFIID ,	   LPVOID *);
		CoCreateInstance = reinterpret_cast < HRESULT(__stdcall *) ( REFCLSID ,LPUNKNOWN ,DWORD,REFIID ,LPVOID *) >(GetProcAddress(GetModuleHandle(TEXT("ole32")),"CoCreateInstance"));


		IShellLink* psl;

		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,IID_IShellLink, (LPVOID*) &psl);

		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;
			hres = psl->QueryInterface( IID_IPersistFile, (LPVOID *) &ppf);
			if (SUCCEEDED(hres))
			{
				hres = ppf->Load((LPOLESTR )LinkFileName.utf16(), STGM_READ);

				if (SUCCEEDED(hres))
				{
					hres = psl->GetArguments(arg, 1024);
				}
				ppf->Release();
			}
			psl->Release();
		}	
		CoUninitialize();
	}
	return QString::fromUtf16((ushort*)arg);
} 
/*
bool getUserLocalFullpath(QSettings* settings,QString filename,QString& dest)
{
	dest = settings->fileName();
	int lastSlash = dest.lastIndexOf(QLatin1Char('/'));
	if (lastSlash == -1)
		return false;
	dest = dest.mid(0, lastSlash);
	dest += "/";
	dest +=filename;
	return true;
}
*/
int getDesktop()
{
	return DESKTOP_WINDOWS;
}

void SetColor(unsigned short ForeColor=FOREGROUND_INTENSITY,unsigned short BackGroundColor=0) 
{ 
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);/*STD_OUTPUT_HANDLE,STD_ERROR_HANDLE*/
	SetConsoleTextAttribute(hCon,ForeColor|BackGroundColor); 
} 
#if 0
quint16 getFileChecksum(QFile *f)
{
	QDataStream ss;
	ss.setDevice(f);
	char buf[1200]={0};
	int readLength=0;
	quint16 checksum=0;
	while(!ss.atEnd())
	{

		readLength=ss.readRawData (buf,1024);
		checksum+=qChecksum(buf,readLength);
	}
	return checksum;

}
#endif
int getFirefoxPath(QString& path)
{
	QString iniPath;
	QString appData;
	QString osPath;

	//#ifdef Q_WS_WIN
	GetShellDir(CSIDL_APPDATA, appData);
	osPath = appData + "/Mozilla/Firefox/";
	//#endif
	iniPath = osPath + "profiles.ini";

	QFile file(iniPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return 0;
	bool isRel = false;
	while (!file.atEnd()) {
		QString line = file.readLine();
		if (line.contains("IsRelative")) {
			QStringList spl = line.split("=");
			isRel = spl[1].toInt();
		}
		if (line.contains("Path")) {
			QStringList spl = line.split("=");
			//if (isRel)
			//	path = osPath + spl[1].mid(0,spl[1].count()-1) + "/bookmarks.html" ;
			//else
			//	path = spl[1].mid(0,spl[1].count()-1);

			path = osPath + spl[1].mid(0,spl[1].count()-1)  ;
			break;
		}
	} 	
	return 1;
}
/*encrypt*/
/*
0    1    2   3    4   5   6   7   8    9    

0    a    b    c    d    e   f   g   h   i    j    

1	l    m    n    o    p   q   r   s   t    u   

2	w    x    y    z    A   B   C   D   E    F   

3	H    I    J    K    L   M   N   O   P    Q   

4   S    T    U    V    W   X   Y   Z   0    1   

5	3	 4	  5	   6    7   8   9   ~   `    !   

6   #    $    %    ^    &   *   (   )   -    -    

7   =    |    \    {    }   [   ]   :   ;    "    

8   <    >    ,    .    ?   /   k   v   G    R   

9	2	 @	 +	   ' 	0   0   0   0   0    0 
*/
uint encrypt_h=10;
uint encrypt_v=10;
uint encrypt_key_index=5;

char encrypt_key[5][10]={
	{'1','a','w','s','4','r','t','5','f','e'},
	{'2','5','t','7','g','8','s','h','b','k'},
	{'4','a','y','w','e','v','6','5','9','m'},
	{'2','r','q','s','4','w','3','5','p','n'},
	{'t','y','w','6','4','l','e','3','f','c'}
};

char encrypt_arr[10][10]={
	{'a','b','c','d','e','f','g','h','i','j'},
	{'l','m','n','o','p','q','r','s','t','u'},
	{'w','x','y','z','A','B','C','D','E','F'}, 
	{'H','I','J','K','L','M','N','O','P','Q'},
	{'S','T','U','V','W','X','Y','Z','0','1'},
	{'3','4','5','6','7','8','9','~','`','!'},
	{'#','$','%','^','&','*','(',')','-','-'},
	{'=','|','\\','{','}','[',']',':',';','\"'},
	{'<','>',',','.','?','/','k','v','G','R'},
	{'2','@','+','\'',' ','0','0','0','0','0'}
};

int getkeylength()
{
	return encrypt_key_index;
}
/*
process the url string:
remove the end char:'/'
example:http://www.sohu.com === http://www.sohu.com/
*/
int handleUrlString(QString& url)
{
	url=url.trimmed();
	while(url.endsWith("/"))
	{
		url.truncate(url.length()-1);
	}
	return 1;
}

uint tz::qhashEx(const QString& str)
{
	uint len=str.length();
	uint h = 0;
	int g=0;
	int i=0;
	while (len--) {
		QChar c=str.at(i++);
		h = ((h) << 4) +c.unicode();
		if ((g = (h & 0xf0000000)) != 0)
			h ^= g >> 23;
		h &= ~g;
	}
	return h;
}
/*
QString tz::getFirefoxBinPath()
{
	QSettings ff_reg("HKEY_LOCAL_MACHINE\\Software\\Mozilla\\Mozilla Firefox",QSettings::NativeFormat);
	qDebug("firefox's version is %s",qPrintable(ff_reg.value("CurrentVersion","").toString()));
	QString firefox_v= ff_reg.value("CurrentVersion","").toString().trimmed();
	ff_reg.beginGroup(firefox_v);
	ff_reg.beginGroup("Main");		 
	QStringList keys = ff_reg.allKeys();
	if(keys.contains("PathToExe",Qt::CaseInsensitive)){
		return ff_reg.value("PathToExe").toString().trimmed();
	}
	return "";
}
*/
/*
QString tz::getIEBinPath()
{
	 QSettings reg("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths",QSettings::NativeFormat);
	 reg.beginGroup("IeXPLORE.EXE");		 
	return reg.value(".").toString().trimmed();
}
*/
uint setLanguage(int l)
{
	language=l;
	if(l<0||(l>=sizeof(gLanguageList)/sizeof(char*)))
		language=DEFAULT_LANGUAGE;
	return 0;	
}

QString tz::encrypt(QString para,uint secindex)
{
	QString out="";
	para=para.trimmed();
	uint len=para.length();
	secindex=secindex%encrypt_key_index;
	uint i=0;
	uint found=0;
	int found_h=-1;
	int found_v=-1;
	while(i<len){
		found=0;
		uint m=0,n=0;
		for(m=0;m<encrypt_h;m++)
		{
			for(n=0;n<encrypt_v;n++ )
			{
				if(encrypt_arr[m][n]==para.at(i).toLatin1())
				{
					found=1;
					found_v=n;
					break;
				}
			}
			if(found)
			{
				found_h=m;
				break;
			}
		}
		if(found)
			out.append(encrypt_key[secindex][found_h]);
		out.append(encrypt_key[secindex][found_v]);
		i++;
	}
	return out;
}
QString tz::decrypt(QString para,uint secindex)
{
	QString out="";
	para=para.trimmed();
	uint len=para.length();
	if(len%2)
		return 1;
	secindex=secindex%encrypt_key_index;
	uint i=0;
	int found_h=-1;
	int found_v=-1;
	while(i<len){
		uint m=0;	
		for(m=0;m<encrypt_h;m++)
		{
			if(encrypt_key[secindex][m]==para.at(i).toLatin1())
			{
				found_h=m;
				break;
			}

		}
		for(m=0;m<encrypt_h;m++)
		{
			if(encrypt_key[secindex][m]==para.at(i+1).toLatin1())
			{
				found_v=m;
				break;
			}

		}
		i=i+2;
		out.append(encrypt_arr[found_h][found_v]);
	}
	return out;
}
QString tz::tr(const char* index)
{
	QString res="unknow error";
	if(QFile::exists(qApp->applicationDirPath()+"/data/language.dat")){
		QSettings langsetting(qApp->applicationDirPath()+"/data/language.dat",QSettings::IniFormat);	
		QByteArray langarray=langsetting.value(QString(index)+"/"+QString(gLanguageList[language]),"unknow error").toByteArray();
		res=QTextCodec::codecForName("UTF-8")->toUnicode(QString(langarray).toLatin1()); 
		res.replace("comma",",");
	}		  	
	return res;
}
struct browserinfo* tz::getbrowserInfo()
{
	return browserInfo;
}
QString tz::getBrowserName(uint id)
{
	int i = 0;
	while(!browserInfo[i].name.isEmpty())
	{
		if(browserInfo[i].id == id)
			return browserInfo[i].name;
		i++;
	}
	return "";
}

void tz::clearbmgarbarge(QSqlQuery* q,uint delId)
{
	//uint comefrom_s=0,comefrom_e=0;
	QString s;
	int i = 0;
	while(!browserInfo[i].name.isEmpty())
	{
		s.clear();
		s=QString("DELETE FROM %1 WHERE comeFrom=%2 and delId!=%3").arg(DBTABLEINFO_NAME(COME_FROM_BROWSER)).arg(BROWSER_ID_TO_COMEFROM(browserInfo[i].id)).arg(delId);
		//qDebug()<<__FUNCTION__<<s;
		q->exec(s);		
		i++;
	}
}
void tz::_clearShortcut(QSqlDatabase *db,int type)
{
	QSqlQuery q("", *db);
	int comefrom_s =0,comefrom_e = 0;
	comefrom_s = comefrom_e = type;
	if(type == COME_FROM_BROWSER){
		comefrom_e = COME_FROM_MAX-1;
	}
	for(int i = comefrom_s; i<=comefrom_e;i++){
		q.prepare(QString("SELECT * FROM %1 WHERE comeFrom=%2").arg(DBTABLEINFO_NAME(COME_FROM_SHORTCUT)).arg(i));
		if(q.exec()){
			QSqlQuery qq("", *db);
			while(q.next()){
				if(!tz::isExistInDb(&qq,Q_VALUE_STRING(q,"shortName"),Q_VALUE_STRING(q,"fullPath"),i))
				{
					qq.prepare(QString("DELETE FROM %1 WHERE id=:id").arg(DBTABLEINFO_NAME(COME_FROM_SHORTCUT)));
					qq.bindValue(":id",Q_VALUE_UINT(q,"id"));
					if(qq.exec())
						qq.clear();					
				}
			}
			q.clear();
		}
	}
}

uint tz::isExistInDb(QSqlQuery* q,const QString& name,const QString& fullpath,int comefrom)
{
	{
		QString queryStr;
		uint id=0;
#if 1
		q->prepare(QString("SELECT id FROM %1 WHERE comeFrom =:comeFrom AND hashId=:hashId AND shortName =:shortName AND fullPath=:fullPath LIMIT 1").arg(DBTABLEINFO_NAME(comefrom)));
		q->bindValue(":comeFrom", comefrom);
		q->bindValue(":hashId", qHash(name));
		q->bindValue(":shortName", name);
		q->bindValue(":fullPath", fullpath);
		q->exec();
		if(q->next())
		{
			//id=q->value(q->record().indexOf("id")).toUInt();
			//id=q->value(Q_PTR_RECORD_INDEX(q,"id")).toUInt();
			id=Q_PTR_VALUE_UINT(q,"id");
		}
		q->clear();

		/*
		q->prepare(QString("select id from "DB_TABLE_NAME" where comeFrom = ? and hashId=? and shortName = ? and fullPath=? limit 1").arg(DBTABLEINFO_NAME(COME_FROM_BROWSER)));
		int i=0;
		q->bindValue(i++, frombrowsertype);
		q->bindValue(i++, qHash(name));
		q->bindValue(i++, name);
		q->bindValue(i++, fullpath);
		q->exec();
		if(q->next())
		{
		id=q->value(q->record().indexOf("id")).toUInt();
		}
		q->clear();
		*/
#else
		queryStr=QString("select id from %1 where comeFrom=%2 and hashId=%3 and shortName='%4' and fullPath='%5' limit 1").arg(DB_TABLE_NAME).arg(frombrowsertype).arg(qHash(name)).arg(name).arg(fullpath);
		qDebug("queryStr=%s",qPrintable(queryStr));
		if(q->exec(queryStr)){
			QSqlRecord rec = q->record();

			int id_Idx = rec.indexOf("id"); // index of the field "name"
			while(q->next()) {	
				id=q->value(id_Idx).toUInt();
				q->clear();
				return id;		
			}						
		}else{
			qDebug("%s query error",__FUNCTION__);
		}
		q->clear();
#endif
		return id;

	}

}
int tz::testFirefoxDbLock(QSqlDatabase* db)
{
	db->setConnectOptions(tr("QSQLITE_BUSY_TIMEOUT=%1").arg(TEST_DB_MAXINUM_TIMEOUT));
	QString queryStr=QString("SELECT * FROM moz_bookmarks LIMIT 1");
	QSqlQuery   query(queryStr, *db);
	if(query.exec()){
		qDebug("test firefox db successfuly!");
		db->setConnectOptions();
		return 1;
	}else{
		qDebug("test firefox db failed!");
		db->setConnectOptions();
		return 0;
	}
}

void tz::addItemToSortlist(const struct bookmark_catagory &bc,QList < bookmark_catagory > *list)
{
	// QDEBUG("add name=%s name_hash=%u",qPrintable(bc.name),bc.name_hash);
	int i=0;
	if(!list->size()) //empty
	{
		list->push_back(bc);
		return;
	}

	for(i=0;i<list->size();i++)
	{
		if(list->at(i).name_hash<bc.name_hash)
			continue;		
		list->insert(i,bc);		
		return;
	}
	if(i==list->size())
	{
		list->push_back(bc);
	}
}
//flag --0--don't check  1---must check
bool tz::readMyBookmark(QSqlDatabase *db, QList < bookmark_catagory > *list,uint level,unsigned int groupid,uint browserid,uint flag=0 )
{
		QSqlQuery q("",*db);
		if(flag&& level>browserInfo[browserid].maxlev)
			return false;
	
		if(flag){
			QString  s=QString("SELECT COUNT(*) as total FROM %1 WHERE  comeFrom=%2 AND  parentid=%3 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK).arg(groupid);
			if(q.exec(s)){
				if(q.next()) {
					if(Q_VALUE_UINT(q,"total")>browserInfo[browserid].maxchild)
						return false;
				}
			}else
				return false;
		}
		
		//process directory
		QString  s=QString("SELECT * FROM %1 WHERE  comeFrom=%2 AND type=1 AND  parentid=%3 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK).arg(groupid);
		if(q.exec(s))
		{
			while(q.next()) {
				struct bookmark_catagory bc;
				bc.name =Q_VALUE_STRING(q,"shortName");
				// dir_bc.name.trimmed();
				bc.name_hash=tz::qhashEx(bc.name);
				bc.link.clear();
				bc.link_hash=0;
				bc.flag = BOOKMARK_CATAGORY_FLAG;
				bc.level = level;
				bc.bmid = Q_VALUE_UINT(q,"id");
				bc.groupId= Q_VALUE_UINT(q,"groupid");
				bc.parentId= Q_VALUE_UINT(q,"parentid");
				readMyBookmark(db,&(bc.list), level + 1,Q_VALUE_UINT(q,"groupid"),browserid,flag);
				addItemToSortlist(bc,list);
			}		
		}else
		return false;
		q.clear();
		//process bookmark
		s=QString("SELECT * FROM %1 WHERE  comeFrom=%2 AND type=0 AND  parentid=%4 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK).arg(groupid);
		if(q.exec(s))
		{
			while(q.next()) {
				struct bookmark_catagory bc;
				bc.name =Q_VALUE_STRING(q,"shortName");
				bc.name.trimmed();
				bc.name_hash=tz::qhashEx(bc.name);
				bc.link = Q_VALUE_STRING(q,"fullPath");
				if( bc.link.isEmpty()) continue;
				QUrl url(bc.link);
				if (!url.isValid() || ((url.scheme().toLower() != QLatin1String("http"))&&(url.scheme().toLower() != QLatin1String("https")))) {
					//qDebug()<<"unvalid http format!";
					continue;
				}
				handleUrlString(bc.link);
				bc.link_hash=tz::qhashEx(bc.link);
				bc.flag = BOOKMARK_ITEM_FLAG;
				bc.level = level;	
				bc.bmid = Q_VALUE_UINT(q,"id");
				bc.groupId= Q_VALUE_UINT(q,"groupid");
				bc.parentId= Q_VALUE_UINT(q,"parentid");
				addItemToSortlist(bc,list);
			}		
		}else
		return false;
		q.clear();
		return true;
}
/*
	flag --0--don't check  1---must check
	
*/
bool tz::readDirectory(QString directory, QList < bookmark_catagory > *list, uint level,uint browserid,uint flag)
{
	QDir qd(directory);
	QString dir = qd.absolutePath();
	QStringList dirs = qd.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	QStringList files = qd.entryList(QStringList("*.url"), QDir::Files, QDir::Unsorted);
	if(flag&& level>browserInfo[browserid].maxlev)
		return false;
	if(flag&&((uint)(dirs.count()+files.count())>browserInfo[browserid].maxchild))
		return false;
	
	for (int i = 0; i < dirs.count(); ++i)
	{
		QString cur = dirs[i];
		if (cur.contains(".lnk"))
			continue;
		struct bookmark_catagory dir_bc;
		dir_bc.name = dirs[i];
		// dir_bc.name.trimmed();
		dir_bc.name_hash=tz::qhashEx(dir_bc.name);
		dir_bc.link.clear();
		dir_bc.link_hash=0;
		dir_bc.flag = BOOKMARK_CATAGORY_FLAG;
		dir_bc.level = level;
		readDirectory(dir + "/" + dirs[i], &(dir_bc.list), level + 1,browserid,flag);
		addItemToSortlist(dir_bc,list);
	}
	
	for (int i = 0; i < files.count(); ++i)
	{
		struct bookmark_catagory bc;
	//	qDebug()<<(dir + "/" + files[i]);
		QSettings favSettings (dir + "/" + files[i], QSettings::IniFormat);
		int dotIndex = files[i].lastIndexOf('.');
		files[i].truncate(dotIndex);
		bc.link = favSettings.value("InternetShortcut/URL").toString();
		if( bc.link.isEmpty()) continue;
		QUrl url(bc.link);
		if (!url.isValid() || ((url.scheme().toLower() != QLatin1String("http"))&&(url.scheme().toLower() != QLatin1String("https")))) {
					//qDebug()<<"unvalid http format!";
					continue;
		}
		handleUrlString(bc.link );
		bc.name = files[i];
		bc.name.trimmed();
		bc.name_hash=tz::qhashEx(bc.name);
		bc.link_hash=tz::qhashEx(bc.link);
		bc.flag = BOOKMARK_ITEM_FLAG;
		bc.level = level;			
		addItemToSortlist(bc,list);
	}
	return true;
}
int tz::getFirefoxVersion()
{
	int ver = 0;

	QSettings ff_reg("HKEY_LOCAL_MACHINE\\Software\\Mozilla\\Mozilla Firefox",QSettings::NativeFormat);
	qDebug("firefox's version is %s",qPrintable(ff_reg.value("CurrentVersion","").toString()));
	QString ff_v= ff_reg.value("CurrentVersion","").toString().trimmed();


	if(!ff_v.isEmpty())
	{
		if(ff_v.at(0).isDigit())
		{
			if(QString(ff_v.at(0)).toInt()>=3)
				ver=FIREFOX_VERSION_3;
			else 
				ver=FIREFOX_VERSION_2;
		}
		else
			ver=0;
	}
	return ver;
}
bool tz::checkFirefoxDir(QString& path)
{
	if(!getFirefoxPath(path))
		return false;
	if(path.isNull()||path.isEmpty())
		return false;
	if(!QFile::exists(path))
		return false;
	return true;
}
bool tz::openFirefox3Db(QSqlDatabase& db,QString path)
{

	db = QSqlDatabase::addDatabase("QSQLITE", "dbFirefox");					
	QString ffpath=QString(path).append("/places.sqlite");
	db.setDatabaseName(ffpath);	
	//qDebug()<<"Open Firefox DB:"<<ffpath;
	if ( !db.open())  {
		// qDebug("connect %s failed",qPrintable(ff_path));     
		goto bad;
	}else{ 
		//qDebug("connect database %s successfully!\n",qPrintable(ff_path));   
		if(!tz::testFirefoxDbLock(&db)){
			qDebug()<<"firefox db is locked!";
			goto bad;
		}
	}
	qDebug("Open Firefox DB successfully!");
	return true;	
bad:
	closeFirefox3Db(db);
	return false;
}
void tz::closeFirefox3Db(QSqlDatabase& db)
{
	if(db.isOpen())	
		db.close();
	QSqlDatabase::removeDatabase("dbFirefox");		
}
/*
QString tz::getIePath()
{
	if(iePath.isEmpty())
		GetShellDir(CSIDL_FAVORITES, iePath);
	return iePath;
}
*/
QString tz::getUserFullpath(QSettings* s,int type)
{
	switch(type){
		case LOCAL_FULLPATH_BMDAT:
			if(localfullpath[type].isEmpty()){
				QString dest = s->fileName();
				int lastSlash = dest.lastIndexOf(QLatin1Char('/'));
				if (lastSlash == -1)
					return QString("");
				dest = dest.mid(0, lastSlash);
				dest += "/";
				dest +=QString(LOCAL_BM_SETTING_FILE_NAME);
				localfullpath[type] = dest;
			}
			return localfullpath[type];
			break;
		case LOCAL_FULLPATH_DB:
			if(localfullpath[type].isEmpty()){
				QString dest = s->fileName();
				int lastSlash = dest.lastIndexOf(QLatin1Char('/'));
				if (lastSlash == -1)
					return QString("");
				dest = dest.mid(0, lastSlash);
				dest += "/";
				dest +=QString(DB_DATABASE_NAME);
				localfullpath[type] = dest;
			}
			return localfullpath[type];
			break;
		case LOCAL_FULLPATH_INI:
			if(localfullpath[type].isEmpty()){
				QString dest = s->fileName();
				int lastSlash = dest.lastIndexOf(QLatin1Char('/'));
				if (lastSlash == -1)
					return QString("");
				dest = dest.mid(0, lastSlash);
				dest += "/";
				dest +=QString(APP_INI_NAME);
				localfullpath[type] = dest;
			}
			return localfullpath[type];
			break;
		case LOCAL_FULLPATH_TEMP:
			if(localfullpath[type].isEmpty()){
				TCHAR tmp[_MAX_PATH]={0};
				GetTempPath(_MAX_PATH,  tmp);
				localfullpath[type]= QString::fromUtf16((const ushort *) tmp);
			}
			return localfullpath[type];
			break;
		case LOCAL_FULLPATH_IEFAV:
			if(localfullpath[type].isEmpty())
				GetShellDir(CSIDL_FAVORITES, localfullpath[type]);
			return localfullpath[type];
			break;
		case LOCAL_FULLPATH_IE:
			if(localfullpath[type].isEmpty()){
				 QSettings reg("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths",QSettings::NativeFormat);
				 reg.beginGroup("IeXPLORE.EXE");		 
				 localfullpath[type]= reg.value(".").toString().trimmed();
			}
			return localfullpath[type];
			break;
		case LOCAL_FULLPATH_FIREFOX:
			if(localfullpath[type].isEmpty()){
				QSettings ff_reg("HKEY_LOCAL_MACHINE\\Software\\Mozilla\\Mozilla Firefox",QSettings::NativeFormat);
				TD(DEBUG_LEVEL_NORMAL,"firefox's version is "<<ff_reg.value("CurrentVersion","").toString());
				QString firefox_v= ff_reg.value("CurrentVersion","").toString().trimmed();
				if(!firefox_v.isEmpty()){
					ff_reg.beginGroup(firefox_v);
					ff_reg.beginGroup("Main");		 
					QStringList keys = ff_reg.allKeys();
					if(keys.contains("PathToExe",Qt::CaseInsensitive)){
						 localfullpath[type]= ff_reg.value("PathToExe").toString().trimmed();
					}				
				}
			}
			return localfullpath[type];
			break;
		case LOCAL_FULLPATH_DEFBROWSER:
			if(localfullpath[type].isEmpty()){
				QString defBrowserPath;
				 HKEY hkRoot,hSubKey; 
				 QString htmlValue = QString("htmlfile");
				 TCHAR defBrowser[MAX_PATH]={0}; 
				 unsigned long cbValueName=MAX_PATH; 
				 unsigned long cbDataValue=MAX_PATH; 
				 DWORD dwType; 
				 TCHAR ValueName[MAX_PATH]={0};

				 if(RegOpenKey(HKEY_CLASSES_ROOT,(LPCTSTR)QString(".html").utf16(),&hkRoot)==ERROR_SUCCESS) 
				 {
				 	TCHAR lpWstr[MAX_PATH]={0};
					DWORD lpType = REG_SZ;
					DWORD maxBufSize = MAX_PATH;
					if(RegQueryValueEx(hkRoot, NULL, NULL, &lpType, (LPBYTE)lpWstr, &maxBufSize) == ERROR_SUCCESS)
						htmlValue=QString::fromUtf16((const ushort *) lpWstr);
					RegCloseKey(hkRoot);
				}
				 
				 if(RegOpenKey(HKEY_CLASSES_ROOT,NULL,&hkRoot)==ERROR_SUCCESS)
				 {
				 	if(RegOpenKeyEx(hkRoot, htmlValue .append(QString("\\shell\\open\\command")).utf16(), 0, KEY_ALL_ACCESS, &hSubKey)==ERROR_SUCCESS) 
						{ 
							RegEnumValue( hSubKey,  0, ValueName, &cbValueName, NULL, &dwType, (LPBYTE)defBrowser, &cbDataValue); 
							RegCloseKey(hkRoot); 
						} 
					RegCloseKey(hkRoot); 
				}
			 	defBrowserPath= QString::fromUtf16((const ushort *) defBrowser);
				defBrowserPath=defBrowserPath.trimmed();
				if(!defBrowserPath.isEmpty()){
					QRegExp regex_path("\"([^\"]*)\"", Qt::CaseInsensitive);
					 int pos = regex_path.indexIn(defBrowserPath);
					 if (pos > -1) {
					     defBrowserPath = regex_path.cap(1); 
					 }
				}
				localfullpath[type]= defBrowserPath;
			}
			return localfullpath[type];
			break;
			
	}
	return QString("");
}
QString tz::getPinyin(const char* s)
{
	if(!s) 
		return "";
	QString r;
	{
		QSqlDatabase db ;
		db= QSqlDatabase::addDatabase("QSQLITE", "pinyindb");
		db.setDatabaseName(APP_DATA_PATH"/"PINYIN_DB_FILENAME);	

		db.open();
		QSqlQuery q("",db);
		r=QString("select pinyin from %1 where hashId=%2 and word='%3' limit 1").arg(PINYIN_DB_TABLENAME).arg(tz::qhashEx(QString(s))).arg(s);
		if(q.exec(r)){					
			while(q.next()) { 
				r = q.value(0).toString();
				//qDebug()<<q.value(0).toString();	
			}
			q.clear();
		}	
		db.close();
	}
	QSqlDatabase::removeDatabase("pinyindb");
	if(r.isEmpty())
		r=QString(s);
	return r;
}
QString tz::fileMd5(const QString& f)
{
	QByteArray  md5res;
	QCryptographicHash md(QCryptographicHash::Md5);
	if(!QFile::exists(f))
	{
		//fprintf(stdout,"file %s doesn't existed!\n",qPrintable(filename));
		return "";
	}
	QFile infile(f);
	if (infile.open(QIODevice::ReadOnly)) {
		QDataStream ins(&infile);
		while(!ins.atEnd())
		{
			char tmp[1024]={0};
			int reads=0;
			if((reads=ins.readRawData(tmp,sizeof(tmp)))!=-1)
			{
				md.addData(tmp,reads);
			}else
			{
				//fprintf(stdout,"read file %s error!\n",qPrintable(filename));
				goto end;		
			}
		}
		md5res = md.result();
		//  fprintf(stdout,"file %s's MD5 is %s!\n",qPrintable(filename),qPrintable(QString(md5res.toHex())));
	}
end:
	if(infile.isOpen())
		infile.close();
	return QString(md5res.toHex());
}
uint tz::registerInt(int mode,const QString& path,const QString&  name,int val)
{
	uint ret = 0;
	QSettings s(path,QSettings::NativeFormat);	
	if(mode == REGISTER_GET_MODE)
	{
		ret = s.value(name,0).toUInt();
	}else{
		s.setValue(name,val);	
		s.sync();
	}
	return ret;
}
QString  tz::registerString(int mode,const QString& path,const QString& name,QString val)
{
	QString ret="";
	QSettings s(path,QSettings::NativeFormat);	
	if(mode == REGISTER_GET_MODE)
	{
		ret = s.value(name,"").toString();
	}else{
		s.setValue(name,val);	
		s.sync();
	}
	return ret;
}
int tz::runParameter(int mode,int type,int ret)
{
	if(type<=RUN_PARAMETER_START||type>=RUN_PARAMETER_END)
		return 0;
	switch(mode)
	{
	case GET_MODE://get
		return runparameters[type];
	case SET_MODE://set
		runparameters[type] = ret;
		break;
	}
	return 0;
}
static uint32 httpunconnectedTimeout = QHTTP_UNCONNECTED_TIME_INTERVAL;
static uint32 httphostlookupTimeout = QHTTP_HOSTLOOKUP_TIME_INTERVAL;
static uint32 httpconnectingTimeout = QHTTP_CONNECTING_TIME_INTERVAL;
static uint32 httpsendingTimeout = QHTTP_SENDING_TIME_INTERVAL;
static uint32 httpreadingTimeout = QHTTP_READING_TIME_INTERVAL;
static uint32 httpconnectedTimeout = QHTTP_CONNECTED_TIME_INTERVAL;
static uint32 httpclosingTimeout = QHTTP_CLOSING_TIME_INTERVAL;

static uint32 monitorTimeout = MONITER_TIME_INTERVAL;
static uint32 testnetTimeout =  TEST_SERVER_TIMEOUT;
static uint32 postitemTimeout = POST_ITEM_TIMEOUT;
static uint32 catalogbuilderTimeout = CATALOG_BUILDER_INTERVAL;
static uint32 silentsyncTimeout = SILENT_SYNC_INTERVAL;
static uint32 diggxmlTimeout = DIGG_XML_INTERVAL;
static uint32 autolearnprocessTimeout = AUTO_LEARN_PROCESS_INTERVAL;
static uint32 httpgetTimeout = HTTP_GET_INTERVAL;
static uint32 httpgetrespondTimeout = HTTP_GET_RESPOND_INTERVAL;
static uint32 httppostTimeout = HTTP_POST_INTERVAL;
static uint32 proxytestTimeout = PROXY_TEST_INTERVAL;

struct parameter_mib{
	uint32 id;
	uint32* val;
	uint32 defval;
	uint32 min;
	uint32 max;	
	QString name;
} PARAMETER_MIB[]={
		{QHTTP_UNCONNECTED,&httpunconnectedTimeout,0,0,0,QString("httpunconnectedTimeout")},
		{QHTTP_HOSTLOOKUP,&httphostlookupTimeout,QHTTP_HOSTLOOKUP_TIME_INTERVAL,QHTTP_HOSTLOOKUP_TIME_INTERVAL_MIN,QHTTP_HOSTLOOKUP_TIME_INTERVAL_MAX,QString("httphostlookupTimeout")},
		{QHTTP_CONNECTING,&httpconnectingTimeout,QHTTP_CONNECTING_TIME_INTERVAL,QHTTP_CONNECTING_TIME_INTERVAL_MIN,QHTTP_CONNECTING_TIME_INTERVAL_MAX,QString("httpconnectingTimeout")},
		{QHTTP_SENDING,&httpsendingTimeout,QHTTP_SENDING_TIME_INTERVAL,QHTTP_SENDING_TIME_INTERVAL_MIN,QHTTP_SENDING_TIME_INTERVAL_MAX,QString("httpsendingTimeout")},
		{QHTTP_READING,&httpreadingTimeout,QHTTP_READING_TIME_INTERVAL,QHTTP_READING_TIME_INTERVAL_MIN,QHTTP_READING_TIME_INTERVAL_MAX,QString("httpreadingTimeout")},
		{QHTTP_CONNECTED,&httpconnectedTimeout,QHTTP_CONNECTED_TIME_INTERVAL,QHTTP_CONNECTED_TIME_INTERVAL_MIN,QHTTP_CONNECTED_TIME_INTERVAL_MAX,QString("httpconnectedTimeout")},
		{QHTTP_CLOSING,&httpclosingTimeout,0,0,0,QString("httpclosingTimeout")},
		{SYS_MONITORTIMEOUT,&monitorTimeout,MONITER_TIME_INTERVAL_MAX,MONITER_TIME_INTERVAL_MIN,MONITER_TIME_INTERVAL_MAX,QString("monitorTimeout")},
		{SYS_TESTNETTIMEOUT,&testnetTimeout,TEST_SERVER_TIMEOUT_MAX,TEST_SERVER_TIMEOUT_MIN,TEST_SERVER_TIMEOUT_MAX,QString("testnetTimeout")},
		{SYS_POSTITEMTIMEOUT,&postitemTimeout,POST_ITEM_TIMEOUT_MAX,POST_ITEM_TIMEOUT_MIN,POST_ITEM_TIMEOUT_MAX,QString("postitemTimeout")},
		{SYS_CATALOGBUILDERTIMEOUT,&catalogbuilderTimeout,CATALOG_BUILDER_INTERVAL_MAX,CATALOG_BUILDER_INTERVAL_MIN,CATALOG_BUILDER_INTERVAL_MAX,QString("catalogbuilderTimeout")},
		{SYS_SILENTSYNCTIMEOUT,&silentsyncTimeout,SILENT_SYNC_INTERVAL_MAX,SILENT_SYNC_INTERVAL_MIN,SILENT_SYNC_INTERVAL_MAX,QString("silentsyncTimeout")},
		{SYS_DIGGXMLTIMEOUT,&diggxmlTimeout,DIGG_XML_INTERVAL_MIN,DIGG_XML_INTERVAL_MIN,DIGG_XML_INTERVAL_MAX,QString("diggxmlTimeout")},
		{SYS_AUTOLEARNPROCESSTIMEOUT,&autolearnprocessTimeout,AUTO_LEARN_PROCESS_INTERVAL_MIN,AUTO_LEARN_PROCESS_INTERVAL_MIN,AUTO_LEARN_PROCESS_INTERVAL_MAX,QString("autolearnprocessTimeout")},
		{SYS_HTTPGETTIMEOUT,&httpgetTimeout,HTTP_GET_INTERVAL,HTTP_GET_INTERVAL_MIN,HTTP_GET_INTERVAL_MAX,QString("httpgetTimeout")},
		{SYS_HTTPGETRESPONDTIMEOUT,&httpgetrespondTimeout,HTTP_GET_RESPOND_INTERVAL,HTTP_GET_RESPOND_INTERVAL_MIN,HTTP_GET_RESPOND_INTERVAL_MAX,QString("httpgetrespondTimeout")},
		{SYS_HTTPPOSTTIMEOUTT,&httppostTimeout,HTTP_POST_INTERVAL,HTTP_POST_INTERVAL_MIN,HTTP_POST_INTERVAL_MAX,QString("httppostTimeout")},
		{SYS_PROXYTESTTIMEOUTT,&proxytestTimeout,PROXY_TEST_INTERVAL,PROXY_TEST_INTERVAL_MIN,PROXY_TEST_INTERVAL_MAX,QString("proxytestTimeout")},
		{0,NULL,0,0,0,QString("")}
};

void tz::setParameterMib(QSettings* s,int id)
{
	int i = 0;
	while(PARAMETER_MIB[i].val){
		if(PARAMETER_MIB[i].id == id){
				(*PARAMETER_MIB[i].val) = s->value(QString("SYS/%1").arg(PARAMETER_MIB[i].name), PARAMETER_MIB[i].defval).toUInt();
				if((*PARAMETER_MIB[i].val) <PARAMETER_MIB[i].min)
					(*PARAMETER_MIB[i].val)  = PARAMETER_MIB[i].min;
				else if((*PARAMETER_MIB[i].val)  > PARAMETER_MIB[i].max )
					(*PARAMETER_MIB[i].val)  = PARAMETER_MIB[i].max;
		}
		i++;
	}
	s->sync();
}
uint32 tz::getParameterMib(int id)
{
	int i = 0;
	while(PARAMETER_MIB[i].val){
		if(PARAMETER_MIB[i].id == id){
			return (*PARAMETER_MIB[i].val);
		}
		i++;
	}
	return 0;
}
void tz::initParameterMib(QSettings* s){
	int i = 0;
	while(PARAMETER_MIB[i].val){
		setParameterMib(s,PARAMETER_MIB[i].id);
		i++;
	}
	
#ifdef TOUCH_ANY_DEBUG
	i=0;
	while(PARAMETER_MIB[i].val){
		TD(DEBUG_LEVEL_NORMAL,PARAMETER_MIB[i].name<<*PARAMETER_MIB[i].val);
		i++;
	}
#endif
}

void tz::netProxy(int mode,QSettings* s,QNetworkProxy** r)
{
	switch(mode)
	{
	case GET_MODE://get
		(*r) = netproxy;
		break;
	case SET_MODE://set
//		if(runParameter(GET_MODE,RUN_PARAMETER_NETPROXY_USING,0))
//			return;
		if(s->value("HttpProxy/proxyEnable", false).toBool())
		{
			if(s->value("HttpProxy/proxyAddress", "").toString().trimmed().isEmpty()||s->value("HttpProxy/proxyPort", 0).toUInt()==0)
				return;
			runParameter(SET_MODE,RUN_PARAMETER_NETPROXY_ENABLE,1);
			if(netproxy ==NULL)
			{
				netproxy=new QNetworkProxy();
				netproxy->setType(QNetworkProxy::HttpProxy);				
			}
			if(netproxy){
				netproxy->setHostName(s->value("HttpProxy/proxyAddress", "").toString());		
				netproxy->setPort(s->value("HttpProxy/proxyPort", 0).toUInt());
				netproxy->setUser(s->value("HttpProxy/proxyUsername", "").toString());
				netproxy->setPassword(tz::decrypt(s->value("HttpProxy/proxyPassword", "").toString(),PASSWORD_ENCRYPT_KEY));
			}
		}else{
			runParameter(SET_MODE,RUN_PARAMETER_NETPROXY_ENABLE,0);
		//	if(netproxy&&!runParameter(GET_MODE,RUN_PARAMETER_NETPROXY_USING,0))
			{
				DELETE_OBJECT(netproxy);
			}
		}		
		break;
	}
	return;
}

int tz::GetCpuUsage()
{ 
	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo; 
	SYSTEM_TIME_INFORMATION SysTimeInfo; 
	SYSTEM_BASIC_INFORMATION SysBaseInfo; 
	double dbIdleTime; 
	double dbSystemTime; 
	LONG status; 
	static LARGE_INTEGER liOldIdleTime = {0,0}; 
	static LARGE_INTEGER liOldSystemTime = {0,0};

	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle(TEXT("ntdll")),"NtQuerySystemInformation");
	if (!NtQuerySystemInformation) 
		return -1;

	// get number of processors in the system 
	status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL); 
	if (status != NO_ERROR) 
		return -1;

	// get new system time 
	status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0); 
	if (status!=NO_ERROR) 
		return -1;

	// get new CPU's idle time 
	status =NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL); 
	if (status != NO_ERROR) 
		return -1;

	// if it's a first call - skip it 
	if (liOldIdleTime.QuadPart != 0) 
	{ 
		// CurrentValue = NewValue - OldValue 
		dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime); 
		dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

		// CurrentCpuIdle = IdleTime / SystemTime 
		dbIdleTime = dbIdleTime / dbSystemTime;

		// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors 
		dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;

	}

	// store new CPU's idle and system time 
	liOldIdleTime = SysPerfInfo.liIdleTime; 
	liOldSystemTime = SysTimeInfo.liKeSystemTime;

	return (int)dbIdleTime;
}
void  tz::initDbTables(QSqlDatabase& db,QSettings *s,bool flag)
{
	int i = 0;	
	while(dbtableInfo[i].id){
		dbtableInfolist<<&dbtableInfo[i];
		i++;
	}	
	if(flag){
		foreach(struct dbtableinfo* info, dbtableInfolist) {
			QString s=QString("DROP TABLE %1").arg(info->name);
			QSqlQuery q(s,db);
			q.exec();	
			s=QString("CREATE TABLE %1 ("
				"id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"groupid INTEGER NOT NULL, "
				"parentid INTEGER NOT NULL, "
				"type INTEGER NOT NULL, "
				"shortName VARCHAR(1024) NOT NULL, "
				"realname VARCHAR(1024) NOT NULL, "
				"alias2 VARCHAR(1024),"	
				"domain VARCHAR(1024),"	
				"time INTEGER NOT NULL,"
				"usage INTEGER NOT NULL,"
				"comeFrom INTEGER NOT NULL, "
				"isHasPinyin INTEGER NOT NULL, "
				"shortCut INTEGER NOT NULL,"
				"hashId INTEGER NOT NULL,"
				"delId INTEGER NOT NULL,"
				"pinyinReg VARCHAR(1024), "
				"allchars VARCHAR(1024), "
				"icon VARCHAR(1024), "					   
				"lowName VARCHAR(1024) NOT NULL, "					  
				"fullPath VARCHAR(1024) NOT NULL, "
				"args VARCHAR(1024))").arg(info->name);
			q=QSqlQuery(s,db);
			q.exec(s);
			q.clear();

		}	
		s->setValue("builddefine",0);
		s->sync();
	}
}
QString tz::getDomain(const QString& fullpath)
{
	//IPV4
	QString path = fullpath.trimmed();
	QString ipv4="^((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))$";
	
	QRegExp  ipv4exp=QRegExp(ipv4,Qt::CaseInsensitive);
	
	if(ipv4exp.indexIn(path)==0)
		return "";
	QStringList slist = path.split(".");
	switch(slist.count()){
		case 0:
		return "";
		case 1:
		return path;
		case 2:
		return slist.at(0);
		default:
		if(slist.at(0)=="www")
			return slist.at(1);
		else{
			QString ret;
			ret.append(slist.at(0));
			ret.append(" ");
			ret.append(slist.at(1));
			return ret;
		}
		
	}
}
struct dbtableinfo* tz::dbTableInfo(uint id)
{
	if(id>COME_FROM_BROWSER)
		id=COME_FROM_BROWSER;
	return &dbtableInfo[id-1];
}

QList<struct dbtableinfo*> tz::dbTableInfoList()
{
	return dbtableInfolist;
}
static QString userIniDir;
QString tz::getUserIniDir(int mode,const QString& s)
{
	switch(mode)
	{
	case GET_MODE://get
		return userIniDir;
	case SET_MODE://set
		userIniDir = s;
		return "";
	}
	return "";
}

char* tz::getstatusstring(int status){
	int j =0 ;
	while(statusStrings[j].status<SYNC_STATUS_MAX){
		if(statusStrings[j].status ==status)
			return statusStrings[j].statusstr;
		j++;
	}
	return "unknown error";
}
/*
0---same 1---newer -1---older
*/

int tz::checkToSetting(QSettings *s,const QString &filename1,QString& md51)
{
	int ret=-1;
	int count = s->beginReadArray(UPDATE_PORTABLE_KEYWORD);
	for (int i = 0; i < count; i++)
	{
		s->setArrayIndex(i);
		QString filename2=s->value("name").toString();
		QString md52=s->value("md5").toString();	
		if(filename1!=filename2) 
			continue;
		if(md52 ==md51)
			ret= 0;
		else{ 
			md51 = md52;
			ret = 1;
		}
		break;
	}
	s->endArray();
	return ret;
}

int tz::checkSilentUpdateSettings(QSettings* src, QSettings* dst,int m)
{
	//merge local with server
	int need = 0;
	int count = src->beginReadArray(UPDATE_PORTABLE_KEYWORD);
	for (int i = 0; i < count; i++)
	{
		src->setArrayIndex(i);
		QString f=src->value("name").toString();
		QString md5=src->value("md5","").toString(); 
		int  flag = checkToSetting(dst,f,md5);
		switch(flag)
		{
		case -1://no found
		case 1://newer
			if(m==SETTING_MERGE_SERVERTOLOCAL)
				md5=src->value("md5","").toString(); //reupdate md5,just md5 from server is valid
			if(((m==SETTING_MERGE_SERVERTOLOCAL)&&(flag==-1))||((m==SETTING_MERGE_LOCALTOSERVER)&&(flag==1)))
			{
				if(
					(!QFile::exists(QString(UPDATE_PORTABLE_DIRECTORY).append(f))||
					(md5!=tz::fileMd5(QString(UPDATE_PORTABLE_DIRECTORY).append(f))))&&
					(!QFile::exists(f)||(md5!=tz::fileMd5(f)))
				  )
				{
					need=1;
					goto end;
				}
			}
			break;
		default:
			break;
		}
	}
end:
	src->endArray();
	return need;
}
//check whether  all update files is downloaded successfully or not
int tz::checkSilentUpdateFiles()
{
	
	if(QFile::exists(QString(UPDATE_PORTABLE_DIRECTORY).append(UPDATE_FILE_NAME))){
		QSettings local(UPDATE_FILE_NAME, QSettings::IniFormat, NULL);
		QSettings server(QString(UPDATE_PORTABLE_DIRECTORY).append(UPDATE_FILE_NAME), QSettings::IniFormat, NULL);
		if(checkSilentUpdateSettings(&local,&server,SETTING_MERGE_LOCALTOSERVER)||checkSilentUpdateSettings(&server,&local,SETTING_MERGE_SERVERTOLOCAL))
			return 0;	
		return 1;
	}
	return 0;
}
/*
QString tz::getSystemTempDir()
{
	TCHAR tmp[_MAX_PATH]={0};
	GetTempPath(_MAX_PATH,  tmp);
	return QString::fromUtf16((const ushort *) tmp);
}
*/
/*
QString tz::getDefaultBrowser()
{
	QString defBrowserPath;
	 HKEY hkRoot,hSubKey; 
	 QString htmlValue = QString("htmlfile");
	 TCHAR defBrowser[MAX_PATH]={0}; 
	 unsigned long cbValueName=MAX_PATH; 
	 unsigned long cbDataValue=MAX_PATH; 
	 DWORD dwType; 
	 TCHAR ValueName[MAX_PATH]={0};

	 if(RegOpenKey(HKEY_CLASSES_ROOT,(LPCTSTR)QString(".html").utf16(),&hkRoot)==ERROR_SUCCESS) 
	 {
	 	TCHAR lpWstr[MAX_PATH]={0};
		DWORD lpType = REG_SZ;
		DWORD maxBufSize = MAX_PATH;
		if(RegQueryValueEx(hkRoot, NULL, NULL, &lpType, (LPBYTE)lpWstr, &maxBufSize) == ERROR_SUCCESS)
			htmlValue=QString::fromUtf16((const ushort *) lpWstr);
		RegCloseKey(hkRoot);
	}
	 
	 if(RegOpenKey(HKEY_CLASSES_ROOT,NULL,&hkRoot)==ERROR_SUCCESS)
	 {
	 	if(RegOpenKeyEx(hkRoot, htmlValue .append(QString("\\shell\\open\\command")).utf16(), 0, KEY_ALL_ACCESS, &hSubKey)==ERROR_SUCCESS) 
			{ 
				RegEnumValue( hSubKey,  0, ValueName, &cbValueName, NULL, &dwType, (LPBYTE)defBrowser, &cbDataValue); 
				RegCloseKey(hkRoot); 
			} 
		RegCloseKey(hkRoot); 
	}
 	defBrowserPath= QString::fromUtf16((const ushort *) defBrowser);
	defBrowserPath=defBrowserPath.trimmed();
	if(!defBrowserPath.isEmpty()){
		QRegExp regex_path("\"([^\"]*)\"", Qt::CaseInsensitive);
		 int pos = regex_path.indexIn(defBrowserPath);
		 if (pos > -1) {
		     defBrowserPath = regex_path.cap(1); 
		 }
	}
	return defBrowserPath;
}
*/
unsigned int tz::getNetBookmarkMaxGroupid(QSqlDatabase *db)
{
	QSqlQuery q("",*db);
	unsigned int maxgroupid = NET_BOOKMARK_GROUPID_START;
	QString  s=QString("SELECT max(groupid) FROM %1 WHERE comeFrom=%2 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK);
	if(q.exec(s))
	{
		if(q.next()) {
			maxgroupid = q.record().value(0).toUInt();
			maxgroupid = maxgroupid?(maxgroupid+1):(NET_BOOKMARK_GROUPID_START);
		}		
	}
	q.clear();
	return maxgroupid; 
}
unsigned int  tz::getBmParentId(QSqlDatabase *db,const int& id)
{
	QSqlQuery q("",*db);
	unsigned int parentid = 0;
	QString  s=QString("SELECT parentid FROM %1 WHERE  comeFrom=%2 AND id=%3 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK).arg(id);
	if(q.exec(s))
	{
		if(q.next()) {
			 parentid = q.value(0).toUInt();
		}		
	}
	q.clear();
	return parentid;
}
unsigned int  tz::getBmidFromGroupId(QSqlDatabase *db,const int& groupid)
{
	QSqlQuery q("",*db);
	unsigned int bmid = 0;
	QString  s=QString("SELECT id FROM %1 WHERE  comeFrom=%2 AND groupid=%3 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK).arg(groupid);
	if(q.exec(s))
	{
		if(q.next()) {
			 bmid = q.value(0).toUInt();
		}		
	}
	q.clear();
	return bmid;
}

unsigned int  tz::getBmGroupId(QSqlDatabase *db,const int& id)
{
	QSqlQuery q("",*db);
	unsigned int groupid = 0;
	QString  s=QString("SELECT groupid FROM %1 WHERE  comeFrom=%2 AND id=%3 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK).arg(id);
	if(q.exec(s))
	{
		if(q.next()) {
			 groupid = q.value(0).toUInt();
		}		
	}
	q.clear();
	return groupid;
}
bool tz::deleteNetworkBookmark(QSqlDatabase *db,unsigned int groupid)
{
	bool ret = false;
	QSqlQuery q("",*db);
	if(groupid==0){		
		QString  s=QString("DELETE  FROM %1 WHERE  comeFrom=%2 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK);
		ret = q.exec(s);
		q.clear();
	}else{
		QString s = QString("DELETE   FROM %1 WHERE  comeFrom=%2 AND TYPE=0 AND parentid=%3 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK).arg(groupid);
		if(ret = q.exec(s)){
			q.clear();
			s=QString("SELECT groupid  FROM %1 WHERE  comeFrom=%2 AND TYPE=1 AND parentid=%3 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK).arg(groupid);		
			if(ret =q.exec(s))
			{
				while(q.next()) {
					deleteNetworkBookmark(db,q.value(0).toUInt());
				}	
				q.clear();
				s = QString("DELETE   FROM %1 WHERE  comeFrom=%2 AND TYPE=1 AND groupid=%3 ").arg(DBTABLEINFO_NAME(COME_FROM_MYBOOKMARK)).arg(COME_FROM_MYBOOKMARK).arg(groupid);
				ret = q.exec(s);
				q.clear();
			}
		}
	}
	return ret;
}
/*
void tz::deleteRelatedFromShortCut(QSqlDatabase *db,QString name,QString fullpath,uint comeFrom)
{
	QSqlQuery q("",*db);
	q.prepare(QString("DELETE FROM %1 WHERE comeFrom =:comeFrom AND hashId=:hashId AND shortName =:shortName AND fullPath=:fullPath").arg(DBTABLEINFO_NAME(COME_FROM_SHORTCUT)));
	q.bindValue(":comeFrom", comeFrom);
	q.bindValue(":hashId", qHash(name));
	q.bindValue(":shortName", name);
	q.bindValue(":fullPath", fullpath);
	q.exec();
	q.clear();
}
*/
void tz::clearBmlist(QList<bookmark_catagory> *l){
		for (int i = 0; i < l->size(); i++)
		{
			if((*l)[i].list.size()){
				clearBmlist(&((*l)[i].list));				
			}			
		}
		l->clear();
}
int tz::checkValidBmlist(QList<bookmark_catagory> *l,uint level,uint browserid){
	static uint totalcount = 0;
	if(!level)
		totalcount =0;
	totalcount += l->size();
#ifdef TOUCH_ANY_DEBUG
	static uint lev = 0;
	static uint levcount = 0;	
	if(!level){
		lev = 0;
		levcount = 0;
		totalcount = 0;
	}
		if((uint)(l->size()) >  levcount)
			levcount = l->size();
		if(level >  lev)
			lev = level;
#endif
		if(totalcount > BROWSER_BM_MAX_TOTAL_COUNT)
			return  -BM_SYNC_FAIL_EXCEED_TOTAL_NUM;
		if((uint)(l->size())>browserInfo[browserid].maxchild)
			return  -BM_SYNC_FAIL_EXCEED_DIR_COUNT;
		if(level>browserInfo[browserid].maxlev)
			return -BM_SYNC_FAIL_EXCEED_LEVEL;
		for (int i = 0; i < l->size(); i++)
		{
			if((*l)[i].list.size()){
				int ret = checkValidBmlist(&((*l)[i].list),level+1,browserid);
				if(ret<0)
					return ret;
			}			
		}
#ifdef TOUCH_ANY_DEBUG
		if(!level)
			TD(DEBUG_LEVEL_BMMERGE," level ="<<lev<<"max count ="<<levcount<<"browserid:"<<browserid<<browserInfo[browserid].maxlev<<browserInfo[browserid].maxchild);
#endif
		return 0;
}
#ifdef TOUCH_ANY_DEBUG
/*
ACTION_LIST_CATALOGBUILD=0,
ACTION_LIST_BOOKMARK_SYNC,
ACTION_LIST_TEST_ACCOUNT,
ACTION_LIST_IMPORT_BOOKMARK,
ACTION_LIST_ADD_NETBOOKMARK_DIR,
ACTION_LIST_MODIFY_NETBOOKMARK_DIR,
ACTION_LIST_DELETE_NETBOOKMARK_DIR,
ACTION_LIST_ADD_NETBOOKMARK_ITEM,
ACTION_LIST_MODIFY_NETBOOKMARK_ITEM,
ACTION_LIST_DELETE_NETBOOKMARK_ITEM,
ACTION_LIST_GET_DIGG_XML
*/
QString actionListName[]={
	QString("build catalog"),
	QString("sync bookmark"),
	QString("test account"),
	QString("import bookmark"),
	QString("add directory to netbookmark"),
	QString("modify directory to netbookmark"),
	QString("delete directory to netbookmark"),
	QString("add item to netbookmark"),
	QString("modify item to netbookmark"),
	QString("delete item to netbookmark"),
	QString("get digg xml from server"),
};
QString tz::getActionListName(int type){
	return actionListName[type];
}
#endif

int tz::deleteDirectory(const QString& path)
{
	QDir dir( QDir::toNativeSeparators(path));
	QString dirPath = dir.absolutePath();
	if(!dir.exists()) return 0;
	QStringList files = dir.entryList(QDir::Files);
	for(int i=0;i<files.size();i++)
	{
		dir.remove(dirPath+ "/"+files[i]);
	}
	QStringList dirs = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	for(int i=0;i<dirs.size();i++)
	{
		deleteDirectory(dirPath+ "/"+dirs[i]);
	}
	dir.rmdir(dirPath);
	return 1;
}

#ifdef CONFIG_AUTO_LEARN_PROCESS
QString tz::getProcessExeFullpath(uint dwPID)
{	
	QString ret="";
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;
	
	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwPID );
	if( hModuleSnap == INVALID_HANDLE_VALUE )
	{
		  return ret;
	}
	
	// Set the size of the structure before using it.
	me32.dwSize = sizeof( MODULEENTRY32 );
	
	// Retrieve information about the first module,
	// and exit if unsuccessful
	if( !Module32First( hModuleSnap, &me32 ) )
	{
	   CloseHandle( hModuleSnap );			// clean the snapshot object
	    return ret;
	}
	/*
	// Now walk the module list of the process,
	// and display information about each module
	do
	{
	  _tprintf( TEXT("\n\n	   MODULE NAME: 	%s"),	me32.szModule );
	  _tprintf( TEXT("\n	 Executable 	= %s"), 	me32.szExePath );
	  _tprintf( TEXT("\n	 Process ID 	= 0x%08X"), 		me32.th32ProcessID );
	  _tprintf( TEXT("\n	 Ref count (g)	= 0x%04X"), 	me32.GlblcntUsage );
	  _tprintf( TEXT("\n	 Ref count (p)	= 0x%04X"), 	me32.ProccntUsage );
	  _tprintf( TEXT("\n	 Base address	= 0x%08X"), (DWORD) me32.modBaseAddr );
	  _tprintf( TEXT("\n	 Base size		= %d"), 			me32.modBaseSize );
	
	} while( Module32Next( hModuleSnap, &me32 ) );
	*/
	ret = QString::fromUtf16(me32.szExePath);
	CloseHandle( hModuleSnap );
	return ret;
}
#endif
