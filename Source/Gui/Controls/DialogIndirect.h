/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#ifndef DIALOGINDIRECT_H
#define DIALOGINDIRECT_H

#include "atlheaders.h"
#include "Core/ServiceLocator.h"
#include "Core/i18n/Translator.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"

template <class T, class TBase = CWindow>
class ATL_NO_VTABLE CDialogIndirectImpl : public CDialogImplBaseT< TBase >
{
public:
#ifdef _DEBUG
    bool m_bModal;
    CDialogIndirectImpl() : m_bModal(false) { }
#endif //_DEBUG
    // modal dialogs
    INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
    {
        ATLASSERT(m_hWnd == NULL);
        _Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
#ifdef _DEBUG
        m_bModal = true;
#endif //_DEBUG
        return ::DialogBoxIndirectParam(_Module.GetResourceInstance(), ((T*)this)->GetTemplate(),
            hWndParent, (DLGPROC)T::StartDialogProc, dwInitParam);
    }
    BOOL EndDialog(INT_PTR nRetCode)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        ATLASSERT(m_bModal);    // must be a modal dialog
        return ::EndDialog(m_hWnd, nRetCode);
    }
    // modeless dialogs
    HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
    {
        ATLASSERT(m_hWnd == NULL);
        _Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
#ifdef _DEBUG
        m_bModal = false;
#endif //_DEBUG
        HWND hWnd = ::CreateDialogIndirectParam(_Module.GetResourceInstance(), ((T*)this)->GetTemplate(),
            hWndParent, (DLGPROC)T::StartDialogProc, dwInitParam);
        ATLASSERT(m_hWnd == hWnd);
        return hWnd;
    }
    // for CComControl
    HWND Create(HWND hWndParent, RECT&, LPARAM dwInitParam = NULL)
    {
        return Create(hWndParent, dwInitParam);
    }
    BOOL DestroyWindow()
    {
        ATLASSERT(::IsWindow(m_hWnd));
        ATLASSERT(!m_bModal);    // must not be a modal dialog
        return ::DestroyWindow(m_hWnd);
    }
};


template <class T, class TBase = CWindow>
class ATL_NO_VTABLE CCustomDialogIndirectImpl : public CDialogIndirectImpl< T > {

public:
    HGLOBAL m_hDlgTemplate = nullptr;
    bool m_bAppWindow = false;
    DLGTEMPLATE* CCustomDialogIndirectImpl::GetTemplate()
    {
        HINSTANCE hInst = _Module.GetResourceInstance();
        HRSRC res = FindResource(hInst, MAKEINTRESOURCE(static_cast<T*>(this)->IDD), RT_DIALOG);
        DLGTEMPLATE* dit = (DLGTEMPLATE*)LockResource(LoadResource(hInst, res));

        size_t sizeDlg = ::SizeofResource(hInst, res);
        m_hDlgTemplate = ::GlobalAlloc(GPTR, sizeDlg);
        if (!m_hDlgTemplate) {
            MessageBox(WinUtils::FormatWindowsErrorMessage(GetLastError()), APPNAME, MB_ICONERROR);
            return nullptr;
        }
        auto pMyDlgTemplate = reinterpret_cast<ATL::_DialogSplitHelper::DLGTEMPLATEEX*>(::GlobalLock(m_hDlgTemplate));
        ::memcpy(pMyDlgTemplate, dit, sizeDlg);

        if (ServiceLocator::instance()->translator()->isRTL()) {
            pMyDlgTemplate->exStyle |= WS_EX_LAYOUTRTL | WS_EX_RTLREADING;
        }

        if (m_bAppWindow) {
            pMyDlgTemplate->exStyle |= WS_EX_APPWINDOW;
        }
        return (DLGTEMPLATE*)pMyDlgTemplate;
    }

    void SetAppWindow(bool appWindow) {
        m_bAppWindow = appWindow;
    }

    ~CCustomDialogIndirectImpl()
    {
        if (m_hDlgTemplate) {
            GlobalUnlock(m_hDlgTemplate);
            GlobalFree(m_hDlgTemplate);
        }
    }

    int LocalizedMessageBox(LPCWSTR lpText, LPCWSTR lpCaption = _T(""), UINT uType = MB_OK) {
        return GuiTools::LocalizedMessageBox(m_hWnd, lpText, lpCaption, uType);
    }
    
};

#endif // DIALOGINDIRECT_H
