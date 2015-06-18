/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#pragma once

#include "atlheaders.h"
#include <atlddx.h>
#include "resource.h"       // main symbols
#include "3rdpart/WinHotkeyCtrl.h"
#include "3rdpart/vkCodes.h"
#include "HotkeySettings.h"
#include "Func/LangClass.h"
// CHotkeyEditor
struct MYHOTKEY
{
    MYHOTKEY()
    {
        keyCode = 0;
        keyModifier = 0;
    }

    CString Serialize() const
    {
        DWORD data = MAKELONG(keyCode, keyModifier);
        CString res;
        res.Format(_T("%u"), data);
        return res;
    }

    bool DeSerialize(const CString data)
    {
        DWORD key = _tstoi(data);
        keyCode = LOWORD(key);
        keyModifier = HIWORD(key);
        return true;
    } 

    bool operator!=( const MYHOTKEY& c)
    {
        return (keyCode != c.keyCode || keyModifier != c.keyModifier);
    }

    CString getKeyName(UINT vk, BOOL fExtended)
    {
        LONG lScan = MapVirtualKey(vk, 0) << 16;

        // if it's an extended key, add the extended flag
        if (fExtended)
            lScan |= 0x01000000L;

        // CString str;
        TCHAR buf[20];
        int nLen;

        nLen = ::GetKeyNameTextW(lScan, buf, sizeof(buf));
        return buf;  // str.c_str();
    }

    ACCEL toAccel()
    {
        ACCEL result;
        result.fVirt = 0;
        result.cmd = 0;
        if (keyModifier & MOD_ALT)
            result.fVirt |= FALT;
        if (keyModifier & MOD_CONTROL)
            result.fVirt |= FCONTROL;
        if (keyModifier & MOD_SHIFT)
            result.fVirt |= FSHIFT;
        result.fVirt |= FVIRTKEY;
        result.key = keyCode;
        return result;
    }

    CString toString()
    {
        CString res;
        HotkeyToString(keyCode, keyModifier, res );
        return res;
        /*CString strKeyName;
        WORD wCode = keyCode;
        WORD wModifiers = keyModifier;
        if (!wCode)
            return strKeyName;
        if (wCode != 0 || wModifiers != 0)
        {
            if (wModifiers & MOD_WIN)
            {
                strKeyName += _T("Win");
                strKeyName += L"+";
            }
            if (wModifiers & MOD_CONTROL)
            {
                strKeyName += getKeyName(VK_CONTROL, FALSE);
                strKeyName += L"+";
            }

            if (wModifiers & MOD_SHIFT)
            {
                strKeyName += getKeyName(VK_SHIFT, FALSE);
                strKeyName += L"+";
            }

            if (wModifiers & MOD_ALT)
            {
                strKeyName += getKeyName(VK_MENU, FALSE);
                strKeyName += L"+";
            }

            strKeyName += getKeyName(wCode, wModifiers & HOTKEYF_EXT);
        }
        return strKeyName;*/
    }

    WORD keyCode;
    WORD keyModifier;
};

class CHotkeyItem
{
    public:
        MYHOTKEY localKey;
        MYHOTKEY globalKey;
        CString func;
        DWORD commandId;
        CString name;
        CString GetDisplayName()
        {
            return Lang.GetString(name);
        }

        void Clear()
        {
            localKey.keyCode = 0;
            localKey.keyModifier = 0;
            globalKey.keyCode = 0;
            globalKey.keyModifier = 0;
        }

        bool IsNull() const
        {
            return (localKey.keyCode == 0 && globalKey.keyCode == 0);
        }
};

class CHotkeyEditor :
    public CDialogImpl<CHotkeyEditor>,
    public CWinDataExchange<CHotkeyEditor>
{
    public:
        CHotkeyItem m_data;
        CHotkeyEditor();
        ~CHotkeyEditor();
        CWinHotkeyCtrl localHotkeyCtrl;
        CWinHotkeyCtrl globalHotkeyCtrl;
        enum { IDD = IDD_HOTKEYEDITOR };

        BEGIN_MSG_MAP(CHotkeyEditor)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
        END_MSG_MAP()

        BEGIN_DDX_MAP(CHotkeyEditor)
        END_DDX_MAP()
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};
