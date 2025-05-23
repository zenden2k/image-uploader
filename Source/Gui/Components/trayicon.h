// Implementation of the CNotifyIconData class and the CTrayIconImpl template.
#pragma once

#include <atlmisc.h>
#include "Core/Utils/CoreTypes.h"

// Wrapper class for the Win32 NOTIFYICONDATA structure
class CNotifyIconData : public NOTIFYICONDATA
{
public:	
	CNotifyIconData()
	{
		memset(this, 0, sizeof(NOTIFYICONDATA));
		cbSize = sizeof(NOTIFYICONDATA);
	}
};

// Template used to support adding an icon to the taskbar.
// This class will maintain a taskbar icon and associated context menu.
template <class T>
class CTrayIconImpl
{
private:
	
	CNotifyIconData m_nid;
	bool m_bInstalled;
	UINT m_nDefault;
    bool m_useGuid;

public:	
	UINT WM_TRAYICON;
    CTrayIconImpl()
        : m_bInstalled(false)
        , m_nDefault(0)
        , m_useGuid(false)
	{
		WM_TRAYICON = ::RegisterWindowMessage(_T("WM_TRAYICON"));
	}
	
	virtual ~CTrayIconImpl()
	{
		// Remove the icon
		RemoveIcon();
	}

	// Install a taskbar icon
	// 	lpszToolTip 	- The tooltip to display
	//	hIcon 		- The icon to display
	// 	nID		- The resource ID of the context menu
	/// returns true on success
        bool InstallIcon(LPCTSTR lpszToolTip, HICON hIcon, HMENU menu = NULL, const GUID* pGuid = nullptr)
	{
		T* pT = static_cast<T*>(this);
		// Fill in the data		
		m_nid.hWnd = pT->m_hWnd;
		//m_nid.uID = nID;
		m_nid.hIcon = hIcon;
		m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        if (pGuid) {
            m_useGuid = true;
            m_nid.uFlags |= NIF_GUID;
            m_nid.guidItem = *pGuid;
        }
		m_nid.uCallbackMessage = WM_TRAYICON;
		_tcscpy(m_nid.szTip, lpszToolTip);
//		 m_hTrayIconMenu = menu;
		// Install
		m_bInstalled = Shell_NotifyIcon(NIM_ADD, &m_nid) ? true : false;
		// Done
		return m_bInstalled;
	}

	// Remove taskbar icon
	// returns true on success
	bool RemoveIcon()
	{
		if (!m_bInstalled)
			return false;

        if (m_useGuid) {
            m_nid.uFlags = NIF_GUID;
        } else {
            m_nid.uFlags = 0;
        }
		// Remove

		return Shell_NotifyIcon(NIM_DELETE, &m_nid) ? true : false;
	}

    bool UpdateIcon(HICON hIcon) 
    {
        T* pT = static_cast<T*>(this);
        // Fill in the data		
        m_nid.hWnd = pT->m_hWnd;
        m_nid.hIcon = hIcon;
        m_nid.uFlags = NIF_ICON;
        if (m_useGuid) {
            m_nid.uFlags |= NIF_GUID;
        } 
        m_nid.uCallbackMessage = WM_TRAYICON;
        return Shell_NotifyIcon(NIM_MODIFY, &m_nid) != FALSE;
	}

	// Set the icon tooltip text
	// returns true on success
	bool SetTooltipText(LPCTSTR pszTooltipText)
	{
		if (pszTooltipText == NULL)
			return FALSE;
		// Fill the structure
		m_nid.uFlags = NIF_TIP;
        if (m_useGuid) {
            m_nid.uFlags |= NIF_GUID;
        } 
		_tcscpy(m_nid.szTip, pszTooltipText);
		// Set
		return Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
	}

	// Set the default menu item ID
	inline void SetDefaultItem(UINT nID) { m_nDefault = nID; }

    void ShowBaloonTip(const CString& text, const CString& title, unsigned int timeout)
    {
        T* pT = static_cast<T*>(this);
        NOTIFYICONDATA nid;
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = /*NOTIFYICONDATA_V2_SIZE*/sizeof(NOTIFYICONDATA);
        nid.hWnd = pT->m_hWnd;
        nid.uTimeout = timeout;
        nid.uFlags = NIF_INFO;
        if (m_useGuid) {
            nid.uFlags |= NIF_GUID;
            nid.guidItem = m_nid.guidItem;
        } 
        nid.dwInfoFlags = NIIF_INFO;
        lstrcpyn(nid.szInfo, text, ARRAY_SIZE(nid.szInfo) - 1);
        lstrcpyn(nid.szInfoTitle, title, ARRAY_SIZE(nid.szInfoTitle) - 1);
        Shell_NotifyIcon(NIM_MODIFY, &nid);
    }

	// Allow the menu items to be enabled/checked/etc.
	virtual void PrepareMenu(HMENU hMenu)
	{
		// Stub
	}
};
