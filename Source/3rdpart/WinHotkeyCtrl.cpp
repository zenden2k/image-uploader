// WinHotkeyCtrl.cpp : implementation file
//

#include "WinHotkeyCtrl.h"
#include "vkCodes.h"
#include "3rdpart/MemberFunctionCallback.h"
#include "Core/i18n/Translator.h"

// CWinHotkeyCtrl

HHOOK CWinHotkeyCtrl::sm_hhookKb = NULL;
CWinHotkeyCtrl* CWinHotkeyCtrl::sm_pwhcFocus = NULL;

CWinHotkeyCtrl::CWinHotkeyCtrl()  
    : m_vkCode(0), m_fModSet(0), m_fModRel(0), m_fIsPressed(FALSE), 
      m_callback(std::bind(&CWinHotkeyCtrl::LowLevelKeyboardProc, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)){
}

CWinHotkeyCtrl::~CWinHotkeyCtrl() {
}

// CWinHotkeyCtrl

void CWinHotkeyCtrl::PreSubclassWindow()
{
    //CEdit::PreSubclassWindow();
    UpdateText();
}


#if _WIN32_WINNT < 0x500

LRESULT CALLBACK CWinHotkeyCtrl::KeyboardProc(
    int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode == HC_ACTION && sm_pwhcFocus) {
        sm_pwhcFocus->PostMessage(WM_KEY, wParam, (lParam & 0x80000000));
    }
    return(1);
}

#else // _WIN32_WINNT >= 0x500

LRESULT /*CALLBACK*/ CWinHotkeyCtrl::LowLevelKeyboardProc(
    int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode < 0)
    {
        return CallNextHookEx(sm_hhookKb, nCode, wParam, lParam);
    }
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN ||
        wParam == WM_KEYUP || wParam == WM_SYSKEYUP) && sm_pwhcFocus) {
            sm_pwhcFocus->PostMessage(WM_KEY, 
                ((PKBDLLHOOKSTRUCT)lParam)->vkCode, (wParam & 1));
            return(1);
           
    }
    
    return CallNextHookEx(sm_hhookKb, nCode, wParam, lParam);
}

#endif // _WIN32_WINNT >= 0x500


BOOL CWinHotkeyCtrl::InstallKbHook() {

    if (sm_pwhcFocus && sm_hhookKb)
        sm_pwhcFocus->UninstallKbHook();
    sm_pwhcFocus = this;

#if _WIN32_WINNT < 0x500
    sm_hhookKb = ::SetWindowsHookEx(WH_KEYBOARD, 
        (HOOKPROC)KeyboardProc, NULL, GetCurrentThreadId());
#else // _WIN32_WINNT >= 0x500
    sm_hhookKb = ::SetWindowsHookEx(WH_KEYBOARD_LL, 
        (HOOKPROC)m_callback, GetModuleHandle(NULL), NULL);
#endif // _WIN32_WINNT >= 0x500

    return(sm_hhookKb != NULL);
}

BOOL CWinHotkeyCtrl::UninstallKbHook() {

    BOOL fOk = FALSE;
    if (sm_hhookKb) {
        fOk = ::UnhookWindowsHookEx(sm_hhookKb);
        sm_hhookKb = NULL;
    }
    sm_pwhcFocus = NULL;
    return(fOk);
}


void CWinHotkeyCtrl::UpdateText() {

    CString sText;
    if (HotkeyToString(m_vkCode, m_fModSet, sText))
        SetWindowText((LPCTSTR)sText);
    else SetWindowText(_T("None"));
    SetSel(0x8fffffff, 0x8fffffff, FALSE);
}

DWORD CWinHotkeyCtrl::GetWinHotkey() {
    return(MAKEWORD(m_vkCode, m_fModSet));
}

BOOL CWinHotkeyCtrl::GetWinHotkey(UINT* pvkCode, UINT* pfModifiers) {
    *pvkCode = m_vkCode;
    *pfModifiers = m_fModSet;
    return(m_vkCode != 0);
}

void CWinHotkeyCtrl::SetWinHotkey(DWORD dwHk) {
    m_vkCode = LOBYTE(LOWORD(dwHk));
    m_fModSet = m_fModRel = HIBYTE(LOWORD(dwHk));
    m_fIsPressed = FALSE;
    UpdateText();
}

void CWinHotkeyCtrl::SetWinHotkey(UINT vkCode, UINT fModifiers) {
    m_vkCode = vkCode;
    m_fModSet = m_fModRel = fModifiers;
    m_fIsPressed = FALSE;
    UpdateText();
}


void CWinHotkeyCtrl::Clear()
{
    m_vkCode = m_fModSet = m_fModRel = 0;
    m_fIsPressed = FALSE;
    UpdateText();
}

LRESULT CWinHotkeyCtrl::OnKey(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    DWORD fMod = 0;
    BOOL fRedraw = TRUE;

    switch (wParam) {
        case VK_LWIN:
        case VK_RWIN: fMod = MOD_WIN; break;
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL: fMod = MOD_CONTROL; break;
        case VK_MENU:
        case VK_LMENU: 
        case VK_RMENU: fMod = MOD_ALT; break;
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT: fMod = MOD_SHIFT; break;
    }

    if (fMod) { // modifier
        if (!lParam) { // press
            if (!m_fIsPressed && m_vkCode) {
                m_fModSet = m_fModRel = 0;
                m_vkCode = 0;
            } 
            m_fModRel &= ~fMod;
        } else if (m_fModSet & fMod) // release
            m_fModRel |= fMod;

        if (m_fIsPressed || !m_vkCode) {
            if(!lParam) { // press
                if(!(m_fModSet & fMod)) { // new modifier
                    m_fModSet |= fMod;
                } else
                    fRedraw = FALSE;
            } else m_fModSet &= ~fMod;
        }
    } else { // another key

        if (wParam == VK_DELETE && m_fModSet == (MOD_CONTROL | MOD_ALT)) {
            m_fModSet = m_fModRel = 0; // skip "Ctrl + Alt + Del"
            m_vkCode = 0;
            m_fIsPressed = FALSE;
        } else if (wParam == m_vkCode && lParam) {
            m_fIsPressed = FALSE;
            fRedraw = FALSE;
        } else {
            if (!m_fIsPressed && !lParam) { // pressed a another key
                if (m_fModRel & m_fModSet) {
                    m_fModSet = m_fModRel = 0;
                }
                m_vkCode = (UINT)wParam;
                m_fIsPressed = TRUE;
            }
        }
    }
    if (fRedraw)
        UpdateText();

    return(0L);
}

// CWinHotkeyCtrl message handlers

void CWinHotkeyCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
}

BOOL CWinHotkeyCtrl::OnSetCursor(HWND pWnd, UINT nHitTest, UINT message) {
    return(FALSE);
}

LRESULT CWinHotkeyCtrl::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = false;
    InstallKbHook();
    return 0;
}

LRESULT CWinHotkeyCtrl::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = false;
    UninstallKbHook();
    return 0;
}

void CWinHotkeyCtrl::OnContextMenu(HWND /*pWnd*/, CPoint pt) {

    UINT id;
    HMENU hmenu, hmenu2;
    hmenu = CreatePopupMenu();
    
#if _WIN32_WINNT >= 0x500
    hmenu2 = CreatePopupMenu();
    for (id = VK_BROWSER_BACK; id <= VK_LAUNCH_APP2; id++)
        AppendMenu(hmenu2, MF_STRING, id, GetKeyName(id));
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hmenu2, _T("Multimedia"));
#endif // _WIN32_WINNT >= 0x500

    hmenu2 = CreatePopupMenu();
    AppendMenu(hmenu2, MF_STRING, VK_RETURN, GetKeyName(VK_RETURN));
    AppendMenu(hmenu2, MF_STRING, VK_ESCAPE, GetKeyName(VK_ESCAPE));
    AppendMenu(hmenu2, MF_STRING, VK_TAB, GetKeyName(VK_TAB));
    AppendMenu(hmenu2, MF_STRING, VK_CAPITAL, GetKeyName(VK_CAPITAL));
    AppendMenu(hmenu2, MF_STRING, VK_BACK, GetKeyName(VK_BACK));
    AppendMenu(hmenu2, MF_STRING, VK_INSERT, GetKeyName(VK_INSERT));
    AppendMenu(hmenu2, MF_STRING, VK_DELETE, GetKeyName(VK_DELETE));
    for (id = VK_SPACE; id <= VK_DOWN; id++)
        AppendMenu(hmenu2, MF_STRING, id, GetKeyName(id));
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hmenu2, _T("Standard"));

    hmenu2 = CreatePopupMenu();
    for (id = VK_F1; id <= VK_F24; id++)
        AppendMenu(hmenu2, MF_STRING, id, GetKeyName(id));
    
    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)hmenu2, _T("Functionality"));


    AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hmenu, (m_fModSet & MOD_WIN) ? 
        (MF_STRING | MF_CHECKED) : MF_STRING, VK_LWIN, _T("Win-key"));
    AppendMenu(hmenu, (m_fModSet & MOD_CONTROL) ? 
        (MF_STRING | MF_CHECKED) : MF_STRING, VK_CONTROL, _T("Control-key"));
    AppendMenu(hmenu, (m_fModSet & MOD_SHIFT) ? 
        (MF_STRING | MF_CHECKED) : MF_STRING, VK_SHIFT, _T("Shift-key"));
    AppendMenu(hmenu, (m_fModSet & MOD_ALT) ? 
        (MF_STRING | MF_CHECKED) : MF_STRING, VK_MENU, _T("Alt-key"));
    AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hmenu, MF_STRING, VK_SNAPSHOT, GetKeyName(VK_SNAPSHOT));
    AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hmenu, MF_STRING, IDM_CLEARHOTKEYDATA, TR("Clear"));
    
    UINT uMenuID = TrackPopupMenu(hmenu, 
        TPM_RIGHTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
        pt.x, pt.y, 0, m_hWnd, NULL);

    if (uMenuID && uMenuID < 256) {
        switch (uMenuID) {
            case VK_LWIN:
                if (m_vkCode) {
                    m_fModSet ^= MOD_WIN;
                    m_fModRel |= m_fModSet & MOD_WIN;
                }
                break;
            case VK_CONTROL:
                if (m_vkCode) {
                    m_fModSet ^= MOD_CONTROL;
                    m_fModRel |= m_fModSet & MOD_CONTROL;
                }
                break;
            case VK_SHIFT:
                if (m_vkCode) {
                    m_fModSet ^= MOD_SHIFT;
                    m_fModRel |= m_fModSet & MOD_SHIFT;
                }
                break;
            case VK_MENU:
                if (m_vkCode) {
                    m_fModSet ^= MOD_ALT;
                    m_fModRel |= m_fModSet & MOD_ALT;
                }
                break;
            case IDM_CLEARHOTKEYDATA:
                Clear();
                break;
            default:
                m_vkCode = uMenuID;
                m_fIsPressed = FALSE;
                break;
        }
        UpdateText();
        SetFocus();
    }
    DestroyMenu(hmenu);
}

LRESULT CWinHotkeyCtrl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = false;
    //if (sm_pwhcFocus == this)
        sm_pwhcFocus->UninstallKbHook();
    return 0;
}
