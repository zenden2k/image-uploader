// WTLBrowserView.cpp : implementation of the CWTLBrowserView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "WTLBrowserView.h"
#include <Core/Logging.h>

BOOL CWTLBrowserView::PreTranslateMessage(MSG* pMsg)
{
	if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
	   (pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
		return FALSE;

	// give HTML page a chance to translate this message
	return (BOOL)SendMessage(WM_FORWARDMSG, 0, (LPARAM)pMsg);
}


void CWTLBrowserView::OnStatusTextChange(const String& szText)
{
	//m_StatusBar.SetPaneText(ID_DEFAULT_PANE,szText);
}

void CWTLBrowserView::OnProgressChange(long nProgress, long nProgressMax)
{
	CString szText;
	if (nProgressMax>0)
		szText.Format(_T("%d%%"),nProgress*100/nProgressMax);
}

void CWTLBrowserView::OnSetSecureLockIcon(long nSecureLockIcon)
{
	m_bSecured=nSecureLockIcon>0;
}

void CWTLBrowserView::OnNavigateComplete2(IDispatch* pDisp, const CString& szURL)
{
	if ( onNavigateComplete2 ) {
		onNavigateComplete2(szURL);
	}
}

BOOL CWTLBrowserView::OnNavigateError(IDispatch* pDisp, const String& szURL, const String& szTargetFrameName, LONG nStatusCode)
{
	if ( onNavigateError ) {
		return onNavigateError(szURL, nStatusCode);
	}
	return FALSE;
}

void CWTLBrowserView::OnPropertyChange(const String& szProperty)
{
	LOG(ERROR) << szProperty;
}
