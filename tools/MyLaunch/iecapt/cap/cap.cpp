// cap.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "cap.h"

////////////////////////////////////////////////////////////////////
//
// IECapt - A Internet Explorer Web Page Rendering Capture Utility
//
// Copyright (C) 2003-2008 Bjoern Hoehrmann <bjoern@hoehrmann.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id: IECapt.cpp,v 1.10 2008/12/23 11:44:55 hoehrmann Exp $
//
////////////////////////////////////////////////////////////////////

#define VC_EXTRALEAN
#include <stdlib.h>
#include <windows.h>
#include <mshtml.h>
#include <exdispid.h>
#include <gdiplus.h>
#include <atlbase.h>
#include <atlwin.h>
#include <atlcom.h>
#include <atlhost.h>
#include <atlimage.h>

#undef VC_EXTRALEAN

#define ID_TIMEOUTTIMER 0x1101
#define ID_DELAYTIMER   0x1102

class CMain;
class CEventSink;
BOOL isFinish = FALSE;
#pragma comment(lib, "atl.lib") 
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) 
//////////////////////////////////////////////////////////////////
// CEventSink
//////////////////////////////////////////////////////////////////
class CEventSink :
	public CComObjectRootEx <CComSingleThreadModel>,
	public IDispatch
{
public:
	CEventSink() : m_pMain(NULL) {}

	BEGIN_COM_MAP(CEventSink)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY_IID(DIID_DWebBrowserEvents2, IDispatch)
	END_COM_MAP()

	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
	STDMETHOD(Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams,
		VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

public:
	CMain* m_pMain;
};

//////////////////////////////////////////////////////////////////
// CMain
//////////////////////////////////////////////////////////////////
class CMain :
	public CWindowImpl <CMain>
{
	friend CEventSink;

public:

	CMain(LPTSTR uri, LPTSTR file, UINT delay, BOOL silent) : m_uDelay(delay),
		m_dwCookie(0), m_URI(uri), m_fileName(file), m_bSilent(silent) { 
			fileCreated=0;
			m_hwndWebBrowser = NULL;
			m_pWebBrowserUnk = NULL;
			m_pWebBrowser = NULL;
			m_pEventSink =NULL;
			isFinish =FALSE;
	}

	BEGIN_MSG_MAP(CMainWindow)
		MESSAGE_HANDLER(WM_CREATE,  OnCreate)
		MESSAGE_HANDLER(WM_SIZE,    OnSize)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_TIMER,   OnTimer)
	END_MSG_MAP()

	LRESULT OnCreate  (UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize    (UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy (UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer   (UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	BOOL SaveSnapshot(void);
	BOOL DelayedSnapshot(void);
	HRESULT GetMetaCollection(IHTMLElementCollection**   pCollec,   IHTMLDocument2*   pDocument) ;
	HRESULT CheckMetaTags(IHTMLDocument2*   pDocument) ;

public:
	CFile   file;
	UINT    fileCreated;
private:
	LPTSTR m_URI;
	LPTSTR m_fileName;
	BOOL   m_bSilent;
	UINT   m_uDelay;
	UINT   m_nIDEvent;

protected:
	CComPtr<IUnknown> m_pWebBrowserUnk;
	CComPtr<IWebBrowser2> m_pWebBrowser;
	CComObject<CEventSink>* m_pEventSink;
	HWND m_hwndWebBrowser;
	DWORD m_dwCookie;
};

//////////////////////////////////////////////////////////////////
// Implementation of CEventSink
//////////////////////////////////////////////////////////////////
STDMETHODIMP CEventSink::GetTypeInfoCount(UINT* pctinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEventSink::GetTypeInfo(UINT itinfo, LCID lcid,
									 ITypeInfo** pptinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEventSink::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames,
									   UINT cNames, LCID lcid,
									   DISPID* rgdispid)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEventSink::Invoke(DISPID dispid, REFIID riid, LCID lcid,
								WORD wFlags, DISPPARAMS* pdispparams,
								VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
								UINT* puArgErr)
{
	switch (dispid) {
case DISPID_DOCUMENTCOMPLETE: {
	IDispatch* pDisp = pdispparams->rgvarg[1].pdispVal;

	if (!m_pMain->m_pWebBrowser.IsEqualObject(pDisp))
		break;
	printf("%s %d web loading successfully!\n",__FUNCTION__,__LINE__);
	BOOL bSuccess = m_pMain->DelayedSnapshot();
	break;
							  }
case DISPID_NAVIGATEERROR: {
	IDispatch* pDisp = pdispparams->rgvarg[4].pdispVal;

	if (!m_pMain->m_pWebBrowser.IsEqualObject(pDisp))
		break;

	// A navigation error occured in the main frame, so abort.
	// TODO: perhaps this should generate some error message.
	m_pMain->KillTimer(ID_TIMEOUTTIMER);
	m_pMain->PostMessage(WM_CLOSE);
	break;
   }
default:
	break;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////
// Implementation of CMain Messages
//////////////////////////////////////////////////////////////////
LRESULT CMain::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HRESULT hr;
	RECT old;
	IUnknown * pUnk = NULL;
	GetClientRect(&old);

	m_hwndWebBrowser = ::CreateWindow(_T("AtlAxWin80"), m_URI,
		WS_CHILD|WS_DISABLED, old.top, old.left, old.right,
		old.bottom, m_hWnd, NULL, ::GetModuleHandle(NULL), NULL);

	// TODO: Quit if m_hwndWebBrowser is null.
	if(!m_hwndWebBrowser){
		CString errBuf;
		errBuf.Format("error=%d",GetLastError());
		MessageBox(errBuf.GetBuffer(errBuf.GetLength()),_T("cap"),MB_OK);
		errBuf.ReleaseBuffer();
		PostMessage(WM_CLOSE);
		return 1;
	}
	hr = AtlAxGetControl(m_hwndWebBrowser, &m_pWebBrowserUnk);
	if (FAILED(hr))
		return 1;
	if (m_pWebBrowserUnk == NULL)
		return 1;
	hr = m_pWebBrowserUnk->QueryInterface(IID_IWebBrowser2, (void**)&m_pWebBrowser);
	if (FAILED(hr))
		return 1;

	// Set whether it should be silent
	m_pWebBrowser->put_Silent(m_bSilent ? VARIANT_TRUE : VARIANT_FALSE);

	hr = CComObject<CEventSink>::CreateInstance(&m_pEventSink);

	if (FAILED(hr))
		return 1;

	m_pEventSink->m_pMain = this;

	hr = AtlAdvise(m_pWebBrowserUnk, m_pEventSink->GetUnknown(),
		DIID_DWebBrowserEvents2, &m_dwCookie);

	if (FAILED(hr))
		return 1;

	return 0;
}


LRESULT CMain::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_hwndWebBrowser != NULL)
		::MoveWindow(m_hwndWebBrowser, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);

	return 0;
}

LRESULT
CMain::OnTimer(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WPARAM timerid = wParam;
	if (wParam != ID_DELAYTIMER &&
		wParam != ID_TIMEOUTTIMER)
		return 0;
	
	// TODO: should this also clear the other timer?
	KillTimer(wParam);
	if( timerid == ID_DELAYTIMER)
		 SaveSnapshot();
	
	isFinish = TRUE;
	PostQuitMessage(0);
	printf("%s %d\n",__FUNCTION__,__LINE__);
	return 0;
}


LRESULT CMain::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HRESULT hr;

	if (m_dwCookie != 0)
		hr = AtlUnadvise(m_pWebBrowserUnk, DIID_DWebBrowserEvents2, m_dwCookie);

	if(m_pWebBrowser)	m_pWebBrowser.Release();
	if(m_pWebBrowserUnk)m_pWebBrowserUnk.Release();

	PostQuitMessage(0);

	return 0;
}

//////////////////////////////////////////////////////////////////
// Implementation of CMain::DelayedSnapshot
//////////////////////////////////////////////////////////////////
BOOL CMain::DelayedSnapshot()
{
	if (m_uDelay == 0) {
		BOOL bSuccess = SaveSnapshot();
		PostMessage(WM_CLOSE);
		return bSuccess;
	}

	m_nIDEvent = ID_DELAYTIMER;

	if (!SetTimer(m_nIDEvent, m_uDelay))
		PostMessage(WM_CLOSE);

	return 0;
}
HRESULT CMain::GetMetaCollection(IHTMLElementCollection**   pCollec,   IHTMLDocument2*   pDocument)   
{   
	CComPtr<IDispatch> lDispatch;   
	CComQIPtr<IHTMLDocument2> lDocument;   
	HRESULT lResult;       

	lDocument   =   pDocument;  


	CComPtr<IHTMLElementCollection> lCollec;   

	lResult   =   lDocument->get_all(&lCollec);   
	ASSERT(SUCCEEDED(lResult)   &&   lCollec);   

	if   (FAILED(lResult)   ||   (lCollec   ==   NULL))   
		return   E_FAIL; 

	lResult   =   lCollec->tags(CComVariant("META"),   &lDispatch);   
	return   lDispatch->QueryInterface(pCollec);   
} 

HRESULT CMain::CheckMetaTags(IHTMLDocument2*   pDocument)   
{   
	CComQIPtr<IHTMLElementCollection> lCollec;   
	CComQIPtr<IHTMLMetaElement> lMetaElement;   
	CComPtr<IDispatch> lDispatch;   
	HRESULT lResult;   
	long lIndex   =   0;   
	long lLength   =   1;   

	lResult   =   GetMetaCollection(&lCollec,pDocument);

	if   (SUCCEEDED(lResult)   &&   lCollec)   
	{   
		lResult   =   lCollec->get_length(&lLength);   
		ASSERT(SUCCEEDED(lResult));   
	}   

	while   (lIndex   <   lLength)   
	{   
		if   (lCollec)   
		{   
			lResult   =   lCollec->item(CComVariant(lIndex),   CComVariant(),   &lDispatch);   
			lMetaElement   =   lDispatch;   
			lDispatch   =   NULL;   
			ASSERT(SUCCEEDED(lResult)   &&   lMetaElement);   
		}   

		if   (SUCCEEDED(lResult)   &&   lMetaElement)   
		{   
			//   
			CComBSTR lName;   

			lResult   =   lMetaElement->get_name(&lName);   
			if   (SUCCEEDED(lResult)   
				&&   lName   
				&&   (!wcsicmp(CComBSTR(L"Keywords"),   lName)   
				||   !wcsicmp(CComBSTR(L"Description"),   lName)))   
			{   
				CComBSTR lMetaContent;   

				lResult   =   lMetaElement->get_content(&lMetaContent);   
				if   (SUCCEEDED(lResult)   &&   lMetaContent)   
				{   
					BSTR bstrText = lMetaContent.m_str;	
					TCHAR curDir[1024]={0};
					GetCurrentDirectory(1024, curDir);
					if(!fileCreated)
			  {
				  //	  CString filename;
				  // filename.Format(_T("%s.txt"),m_fileName);
				  TCHAR filename[1024]={0};
				  TCHAR temp[1024]={0};
				  _tcscpy(temp,m_fileName);
				  //enter the right directory
				  TCHAR* pszToken = ::_tcstok(temp, _T("/"));
				  TCHAR* rightToken=pszToken;
				  while(NULL != pszToken)
				  {
					  rightToken=pszToken;
					  pszToken = ::_tcstok(NULL, _T("/"));

				  }

				  {
					  WIN32_FIND_DATA     wfd;   
					  HANDLE   hSearch   =  ::FindFirstFile(_T("content"),&wfd);   
					  if   (hSearch   ==   INVALID_HANDLE_VALUE)//无效目录   
					  {   
						  ::CreateDirectory(_T("content"),NULL);
					  }else
						  ::FindClose(hSearch);
					  ::SetCurrentDirectory(_T("content"));
				  }
				  //end to enter the right dirctory
				  _tcscpy(filename,rightToken);
				  _tcscat(filename,_T(".txt"));
				  file.Open(filename,CFile::modeCreate|CFile::modeWrite);    
				  fileCreated=1;
			  }
					CString str(bstrText);
					CString writestr;
					writestr.Format(_T("%s=%s\r\n"), CString(lName.m_str), str);
					TCHAR   *pBuff   =   writestr.GetBuffer(writestr.GetLength());   
					writestr.ReleaseBuffer();   	    
					file.Write(pBuff,   writestr.GetLength());   	
					::SetCurrentDirectory(curDir);
				}   
			}   
		}   

		lIndex++;   
	}   
	if(fileCreated)
	{
		fileCreated=0;
		file.Close(); 
	}
	return   S_OK;   
}   



//////////////////////////////////////////////////////////////////
// Implementation of CMain::SaveSnapshot
//////////////////////////////////////////////////////////////////
BOOL CMain::SaveSnapshot(void)
{
	long bodyHeight, bodyWidth, rootHeight, rootWidth, height, width;
	if(!m_pWebBrowser)
		return TRUE;
	CComPtr<IDispatch> pDispatch;
	HRESULT hr = m_pWebBrowser->get_Document(&pDispatch);
	if (FAILED(hr))
		return true;

	CComPtr<IHTMLDocument2> spDocument;
	hr = pDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&spDocument);

	if (FAILED(hr))
		return true;
	TCHAR curDir[1024]={0};
	GetCurrentDirectory(1024, curDir);
	if(!fileCreated)
	{

		TCHAR filename[1024]={0};
		TCHAR temp[1024]={0};
		_tcscpy(temp,m_fileName);
		TCHAR* pszToken = ::_tcstok(temp, _T("/"));
		TCHAR* rightToken=pszToken;
		while(NULL != pszToken)
		{
			rightToken=pszToken;
			pszToken = ::_tcstok(NULL, _T("/"));

		}
		{
					  WIN32_FIND_DATA     wfd;   
					  HANDLE   hSearch   =  ::FindFirstFile(_T("content"),&wfd);   
					  if   (hSearch   ==   INVALID_HANDLE_VALUE)//无效目录   
					  {   
						  ::CreateDirectory(_T("content"),NULL);
					  }else
						  ::FindClose(hSearch);
					  ::SetCurrentDirectory(_T("content"));
		}
		_tcscpy(filename,rightToken);
		_tcscat(filename,_T( ".txt"));
		file.Open(filename,CFile::modeCreate|CFile::modeWrite);    
		fileCreated=1;
	}
	CComBSTR lTitle;
	hr=spDocument->get_title(&lTitle) ;
	if(SUCCEEDED(hr)){
		CString writestr;
		writestr.Format(_T("title=%s\r\n"), CString(lTitle.m_str));
		TCHAR   *pBuff   =   writestr.GetBuffer(writestr.GetLength());   
		writestr.ReleaseBuffer();   	    
		file.Write(pBuff,   writestr.GetLength());   
	}
	::SetCurrentDirectory(curDir);
	/*add by ramen*/
	CheckMetaTags(spDocument);
	/**/
	CComPtr<IHTMLElement> spBody;
	hr = spDocument->get_body(&spBody);

	if (FAILED(hr))
		return true;

	CComPtr<IHTMLElement2> spBody2;
	hr = spBody->QueryInterface(IID_IHTMLElement2, (void**)&spBody2);

	if (FAILED(hr))
		return true;

	hr = spBody2->get_scrollHeight(&bodyHeight);

	if (FAILED(hr))
		return true;

	hr = spBody2->get_scrollWidth(&bodyWidth);

	if (FAILED(hr))
		return true;

	CComPtr<IHTMLDocument3> spDocument3;
	hr = pDispatch->QueryInterface(IID_IHTMLDocument3, (void**)&spDocument3);

	if (FAILED(hr))
		return true;

	// We also need to get the dimensions from the <html> due to quirks
	// and standards mode differences. Perhaps this should instead check
	// whether we are in quirks mode? How does it work with IE8?
	CComPtr<IHTMLElement> spHtml;
	hr = spDocument3->get_documentElement(&spHtml);

	if (FAILED(hr))
		return true;

	CComPtr<IHTMLElement2> spHtml2;
	hr = spHtml->QueryInterface(IID_IHTMLElement2, (void**)&spHtml2);

	if (FAILED(hr))
		return true;

	hr = spHtml2->get_scrollHeight(&rootHeight);

	if (FAILED(hr))
		return true;

	hr = spHtml2->get_scrollWidth(&rootWidth);

	if (FAILED(hr))
		return true;

	width = bodyWidth;
#if 0
	// height = rootHeight > bodyHeight ? rootHeight : bodyHeight;
#else
	height=(bodyWidth*3)/4;
#endif

	// TODO: What if width or height exceeds 32767? It seems Windows limits
	// the window size, and Internet Explorer does not draw what's not visible.
	::MoveWindow(m_hwndWebBrowser, 0, 0, width, height, TRUE);

	CComPtr<IViewObject2> spViewObject;

	// This used to get the interface from the m_pWebBrowser but that seems
	// to be an undocumented feature, so we get it from the Document instead.
	hr = spDocument3->QueryInterface(IID_IViewObject2, (void**)&spViewObject);

	if (FAILED(hr))
		return true;

	RECTL rcBounds = { 0, 0, width, height };

	_TCHAR* tcsExt = _tcsrchr(m_fileName, '.');
	if (tcsExt && _tcscmp(_T(".emf"), tcsExt) == 0) {

		USES_CONVERSION;
		HDC hdcMain = GetDC();
		int iWidthMM = GetDeviceCaps(hdcMain, HORZSIZE); 
		int iHeightMM = GetDeviceCaps(hdcMain, VERTSIZE); 
		int iWidthPels = GetDeviceCaps(hdcMain, HORZRES); 
		int iHeightPels = GetDeviceCaps(hdcMain, VERTRES); 

		Gdiplus::RectF rcBoundsX(0, 0,
			(Gdiplus::REAL)width, (Gdiplus::REAL)height);

		rcBoundsX.Y      *= iHeightMM * 100 / iHeightPels; 
		rcBoundsX.X      *= iWidthMM  * 100 / iWidthPels; 
		rcBoundsX.Width  *= iWidthMM  * 100 / iWidthPels; 
		rcBoundsX.Height *= iHeightMM * 100 / iHeightPels; 

		Gdiplus::Metafile emf(T2W(m_fileName), hdcMain, rcBoundsX,
			Gdiplus::MetafileFrameUnitGdi, Gdiplus::EmfTypeEmfPlusDual,
			L"[TODO: Description]");

		Gdiplus::Graphics g(&emf);
		HDC imgDc = g.GetHDC();

		// For unknown reasons Internet Explorer will sometimes
		// fail to draw glyphs for certain characters here even
		// though they are rendered in Internet Explorer itself.
		// On other pages, Internet Explorer will simply render
		// a single bitmap into the emf which isn't really what
		// this should do. I've no idea how to fix that however.

		hr = spViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, imgDc,
			imgDc, &rcBounds, NULL, NULL, 0);

		g.ReleaseHDC(imgDc);
		ReleaseDC(hdcMain);
		return false;
	}

	GetCurrentDirectory(1024, curDir);

	TCHAR name_m[1024]={0};
	TCHAR name_b[1024]={0};
	TCHAR name_l[1024]={0};
	//enter the right directory
	TCHAR temp[1024]={0};
	_tcscpy(temp,m_fileName);
	TCHAR* pszToken = ::_tcstok(temp, _T("/"));
	TCHAR* rightToken=pszToken;
	while(NULL != pszToken)
	{
		rightToken=pszToken;

		pszToken = ::_tcstok(NULL, _T("/"));
		if(pszToken)
		{
			WIN32_FIND_DATA     wfd;   
			HANDLE   hSearch   =  ::FindFirstFile(rightToken,&wfd);   
			if   (hSearch   ==   INVALID_HANDLE_VALUE)//无效目录   
			{   
				::CreateDirectory(rightToken,NULL);
			}else
				::FindClose(hSearch);
			::SetCurrentDirectory(rightToken);
		}

	}

	CImage image_orig;
	CImage image_little;
	CImage image_mid;
	CImage image_big;
	//end to enter the right dirctory
	_tcscpy(name_b,_T( "b_"));
	_tcscat(name_b,rightToken);
	_tcscpy(name_l,_T( "l_"));
	_tcscat(name_l,rightToken);
	//_tcscpy(name_l,"l_");
	_tcscpy(name_m,rightToken);

	// TODO: check return value;
	// TODO: somehow enable alpha
#define LITTLE_WIDTH 75
#define LITTLE_HEIGHT 75
#define MIDDLE_WIDTH 112
#define MIDDLE_HEIGHT 83
#define BIG_WIDTH 200
#define BIG_HEIGHT 150
	image_orig.Create(width, height, 24);
	//	printf("width=%d height=%d\n",width,height);

	HDC imgDc = image_orig.GetDC();


	//image_mid 75x75
	image_little.Create(LITTLE_WIDTH, LITTLE_HEIGHT, 24);
	HDC imgDc_little=image_little.GetDC();
	::SetStretchBltMode(imgDc_little,STRETCH_HALFTONE);
	hr = spViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, imgDc,
		imgDc, &rcBounds, NULL, NULL, 0);
	::StretchBlt(imgDc_little,0,0,LITTLE_WIDTH,LITTLE_HEIGHT,imgDc,0,0,width,height,SRCCOPY  );
	::SetBrushOrgEx(imgDc_little, 0, 0, NULL); 
	image_little.ReleaseDC();
	image_little.Save(name_l);


	//image_mid 112x83
	image_mid.Create(MIDDLE_WIDTH, MIDDLE_HEIGHT, 24);
	HDC imgDc_mid=image_mid.GetDC();
	::SetStretchBltMode(imgDc_mid,STRETCH_HALFTONE);
	hr = spViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, imgDc,
		imgDc, &rcBounds, NULL, NULL, 0);
	::StretchBlt(imgDc_mid,0,0,MIDDLE_WIDTH,MIDDLE_HEIGHT,imgDc,0,0,width,height,SRCCOPY  );
	::SetBrushOrgEx(imgDc_mid, 0, 0, NULL); 
	image_mid.ReleaseDC();
	image_mid.Save(name_m);

	//image_big 200x150
	image_big.Create(BIG_WIDTH, BIG_HEIGHT, 24);
	HDC imgDc_big=image_big.GetDC();
	::SetStretchBltMode(imgDc_big,STRETCH_HALFTONE);
	hr = spViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, imgDc,
		imgDc, &rcBounds, NULL, NULL, 0);
	::StretchBlt(imgDc_big,0,0,BIG_WIDTH,BIG_HEIGHT,imgDc,0,0,width,height,SRCCOPY  );
	::SetBrushOrgEx(imgDc_big, 0, 0, NULL); 
	image_big.ReleaseDC();
	image_big.Save(name_b);

	image_orig.ReleaseDC();

	if (SUCCEEDED(hr))
	{
		//hr = image.Save(name_b);
	}
	//	getchar();
	::SetCurrentDirectory(curDir);
	return false;
}

static const GUID myGUID = { 0x445c10c2, 0xa6d4, 0x40a9, { 0x9c, 0x3f, 0x4e, 0x90, 0x42, 0x1d, 0x7e, 0x83 } };
static CComModule _Main;

void
IECaptHelp(void) {
	printf("%s",
		" -----------------------------------------------------------------------------\n"
		" Usage: IECapt --url=http://www.example.org/ --out=localfile.png              \n"
		" -----------------------------------------------------------------------------\n"
		"  --help                      Print this help page and exit                   \n"
		"  --url=<url>                 The URL to capture (http:...|file:...|...)      \n"
		"  --out=<path>                The target file (.png|bmp|jpeg|emf|...)         \n"
		"  --min-width=<int>           Minimal width for the image (default: 800)      \n"
		"  --max-wait=<ms>             Don't wait more than (default: 90000, inf: 0)   \n"
		"  --delay=<ms>                Wait after loading (e.g. for Flash; default: 0) \n"
		"  --silent                    Whether to surpress some dialogs                \n"
		" -----------------------------------------------------------------------------\n"
		" http://iecapt.sf.net - (c) 2003-2008 Bjoern Hoehrmann - <bjoern@hoehrmann.de>\n"
		"");
}

int main (int argc, _TCHAR* argv[])
{
	int ax;
	int argHelp = 0;
	int argSilent = 0;
	_TCHAR* argUrl = NULL;
	_TCHAR* argOut = NULL;
	unsigned int argDelay = 0;
	unsigned int argMinWidth = 800;
	unsigned int argMaxWait = 90000;

	// Parse command line parameters
	for (ax = 1; ax < argc; ++ax) {
		size_t nlen;

		_TCHAR* s = argv[ax];
		_TCHAR* value;

		// boolean options
		if (_tcscmp(_T("--silent"), s) == 0) {
			argSilent = 1;
			continue;

		} else if (_tcscmp(_T("--help"), s) == 0) {
			argHelp = 1;
			break;
		} 
		value = _tcschr(s, '=');
		if (value == NULL) {
			if (argUrl == NULL) {
				argUrl = s;
				continue;
			} else if (argOut == NULL) {
				argOut = s;
				continue;
			}
			// error
			argHelp = 1;
			break;
		}

		nlen = value++ - s;

		// --name=value options
		if (_tcsncmp(_T("--url"), s, nlen) == 0) {
			argUrl = value;
		} else if (_tcsncmp(_T("--min-width"), s, nlen) == 0) {
			// TODO: add error checking here?
			argMinWidth = (unsigned int)_tstoi(value);

		} else if (_tcsncmp(_T("--delay"), s, nlen) == 0) {
			// TODO: add error checking here?
			argDelay = (unsigned int)_tstoi(value);

		} else if (_tcsncmp(_T("--max-wait"), s, nlen) == 0) {
			// TODO: add error checking here?
			argMaxWait = (unsigned int)_tstoi(value);

		} else if (_tcsncmp(_T("--out"), s, nlen) == 0) {
			argOut = value;

		} else {
			// TODO: error
			argHelp = 1;
		}
	}
	CoInitialize( NULL );
	if (argUrl == NULL || argOut == NULL || argHelp) {
		IECaptHelp();
		return EXIT_FAILURE;
	}

	HRESULT hr = _Main.Init(NULL, ::GetModuleHandle(NULL), &myGUID);

	if (FAILED(hr))
		return EXIT_FAILURE;

	if (!AtlAxWinInit())
		return EXIT_FAILURE;

	// Initialize GDI+.
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CMain MainWnd(argUrl, argOut, argDelay, argSilent);

	RECT rcMain = { 0, 0, argMinWidth, 600 };
	MainWnd.Create(NULL, rcMain, _T("IECapt"), WS_POPUP);

	// TODO: decide what to do when max-wait and delay conflict.
	if (argMaxWait != 0)
		MainWnd.SetTimer(ID_TIMEOUTTIMER, argMaxWait);

	MSG msg;
	
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (isFinish)       
				   break ;    		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	Gdiplus::GdiplusShutdown(gdiplusToken);
	CoUninitialize();
	return EXIT_SUCCESS;
}
