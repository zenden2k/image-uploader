/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "TextParamsWindow.h"

#include "resource.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "ImageEditor/Helpers/FontEnumerator.h"
#include <vector>
#include <thread>
#include <memory>

TextParamsWindow::TextParamsWindow() : fontSizeComboboxCustomEdit_(this), windowDc_(nullptr)
{
    memset(&font_, 0, sizeof(font_));
}

TextParamsWindow::~TextParamsWindow()
{
    if ( fontEnumerationThread_.joinable() ) {
        fontEnumerationThread_.join();
    }
}

void TextParamsWindow::setFont(LOGFONT logFont)
{
    fontName_ = logFont.lfFaceName;
    fontComboBox_.SelectString(-1, fontName_);
    CWindowDC dc(m_hWnd);
    SetDlgItemInt(IDC_FONTSIZECOMBO, MulDiv(-logFont.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY)));
    TBBUTTONINFO bi;
    memset(&bi,0,sizeof(bi));
    bi.cbSize = sizeof(bi);
    bi.dwMask = TBIF_STATE;
    bi.fsState = TBSTATE_ENABLED | (logFont.lfWeight ==  FW_BOLD ?  TBSTATE_CHECKED: 0);
    textToolbar_.SetButtonInfo(IDC_BOLD, &bi);
    bi.fsState = TBSTATE_ENABLED | (logFont.lfItalic ? TBSTATE_CHECKED : 0);
    textToolbar_.SetButtonInfo(IDC_ITALIC, &bi);
    bi.fsState = TBSTATE_ENABLED | (logFont.lfUnderline  ? TBSTATE_CHECKED : 0);
    textToolbar_.SetButtonInfo(IDC_UNDERLINE, &bi);
}

LOGFONT TextParamsWindow::getFont()
{    
    LOGFONT logFont;
    memset(&logFont, 0, sizeof(logFont));
    logFont.lfCharSet = DEFAULT_CHARSET;
    CWindowDC dc(m_hWnd);

    int fontSelectedIndex = fontComboBox_.GetCurSel();
    if ( fontSelectedIndex !=-1 ) {
        logFont = fonts_[fontSelectedIndex];
        int fontSize = GetDlgItemInt(IDC_FONTSIZECOMBO);
        logFont.lfHeight =  -MulDiv(fontSize, GetDeviceCaps(dc, LOGPIXELSY), 72);
        TBBUTTONINFO bi;
        memset(&bi,0,sizeof(bi));
        bi.cbSize = sizeof(bi);
        bi.dwMask = TBIF_STATE;
        textToolbar_.GetButtonInfo(IDC_BOLD, &bi);
        logFont.lfWeight = bi.fsState&TBSTATE_CHECKED ? FW_BOLD : 0;
        textToolbar_.GetButtonInfo(IDC_ITALIC, &bi);
        logFont.lfItalic =  bi.fsState&TBSTATE_CHECKED?1:0;
        textToolbar_.GetButtonInfo(IDC_UNDERLINE, &bi);
        logFont.lfUnderline = bi.fsState&TBSTATE_CHECKED?1:0;
    
        /*lf.lfStrikeOut = ( (cf.dwEffects & CFE_STRIKEOUT) == CFE_STRIKEOUT);*/
        
    }
    return logFont;
}

LRESULT TextParamsWindow::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    SetWindowText(TR("Text Options"));
    fontComboBox_.Attach(GetDlgItem(IDC_FONTCOMBO));
    fontSizeComboBox_.Attach(GetDlgItem(IDC_FONTSIZECOMBO));

    fontSizeComboboxCustomEdit_.SubclassWindow(fontSizeComboBox_.GetWindow( GW_CHILD));
	windowDc_ = GetDC();
    auto enumerator = std::make_shared<FontEnumerator>(windowDc_, fonts_, FontEnumerator::OnEnumerationFinishedDelegate(this, &TextParamsWindow::OnFontEnumerationFinished));
    fontEnumerationThread_ = std::thread(&FontEnumerator::run, enumerator);
    GuiTools::AddComboBoxItems(m_hWnd, IDC_FONTSIZECOMBO, 19, _T("7"), _T("8"), _T("9"), _T("10"),_T("11"),_T("12"), _T("13"),
        _T("14"), _T("15"),_T("16"),_T("18"),_T("20"),_T("22"), _T("24"),  _T("26"),  _T("28"),  _T("36"), _T("48"),_T("72")
        );

    RECT toolbarRect;
    HWND toolbarPlaceholder = GetDlgItem(IDC_TOOLBARPLACEHOLDER);
    ::GetWindowRect(toolbarPlaceholder, &toolbarRect);
    ScreenToClient(&toolbarRect);
    int iconWidth =  ::GetSystemMetrics(SM_CXSMICON);
    int iconHeight =  ::GetSystemMetrics(SM_CYSMICON);
    toolbarImageList_.Create(iconWidth, iconHeight, (WinUtils::IsWinXPOrLater()) ?  ILC_COLOR32 : ILC_COLOR8| ILC_MASK,3,3);
    /*if ( WinUtils::IsWinXP() || WinUtils::IsVista() )*/ {
        iconBold_ = GuiTools::LoadSmallIcon(IDI_ICONBOLD);
        toolbarImageList_.AddIcon(iconBold_);
        iconItalic_ = GuiTools::LoadSmallIcon(IDI_ICONITALIC);
        toolbarImageList_.AddIcon(iconItalic_);
        iconUnderline_ = GuiTools::LoadSmallIcon(IDI_ICONUNDERLINE);
        toolbarImageList_.AddIcon(iconUnderline_);
    }
    textToolbar_.Create(m_hWnd, toolbarRect, _T(""), WS_CHILD | TBSTYLE_LIST| /*|TBSTYLE_CUSTOMERASE|*/TBSTYLE_FLAT| CCS_NORESIZE|CCS_BOTTOM | CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );

    textToolbar_.SetButtonStructSize();
    textToolbar_.SetButtonSize(30,18);
    textToolbar_.SetImageList(toolbarImageList_);
    textToolbar_.AddButton(IDC_BOLD, TBSTYLE_CHECK|BTNS_AUTOSIZE ,TBSTATE_ENABLED, 0, 0, 0);
    textToolbar_.AddButton(IDC_ITALIC, TBSTYLE_CHECK|BTNS_AUTOSIZE ,TBSTATE_ENABLED, 1, 0, 0);
    textToolbar_.AddButton(IDC_UNDERLINE, TBSTYLE_CHECK|BTNS_AUTOSIZE ,TBSTATE_ENABLED, 2, 0, 0);
    textToolbar_.ShowWindow(SW_SHOW);

    return TRUE;
}

LRESULT TextParamsWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if (windowDc_) {
		ReleaseDC(windowDc_);
		windowDc_ = nullptr;
	}
	return 0;
}
LRESULT TextParamsWindow::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{    
    ShowWindow(SW_HIDE);
    return 0;
}    

LRESULT TextParamsWindow::OnFontSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    NotifyParent(CFM_FACE);
    return 0;
}

LRESULT TextParamsWindow::OnFontSizeSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int curSel = fontSizeComboBox_.GetCurSel();
    if ( curSel != -1 ) {
        TCHAR fontSize[100]=_T("");
        fontSizeComboBox_.GetLBText(curSel, fontSize);
        SetDlgItemInt(IDC_FONTSIZECOMBO, StrToInt(fontSize));
    }
    NotifyParent(CFM_SIZE);
    return 0;
}

LRESULT TextParamsWindow::OnBoldClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    NotifyParent(CFM_BOLD);
    return 0;
}

LRESULT TextParamsWindow::OnItalicClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    NotifyParent(CFM_ITALIC);
    return 0;
}

LRESULT TextParamsWindow::OnUnderlineClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    NotifyParent(CFM_UNDERLINE);
    return 0;
}

void TextParamsWindow::OnFontEnumerationFinished()
{
    size_t fontCount = fonts_.size();
    for ( size_t i = 0; i < fontCount; i++ ) {
        fontComboBox_.AddString(fonts_[i].lfFaceName);
    }
    fontComboBox_.SelectString(-1, fontName_);
}

void TextParamsWindow::NotifyParent(DWORD changeMask)
{
    ::SendMessage(GetParent(), TPWM_FONTCHANGED, (WPARAM)m_hWnd, (LPARAM)changeMask);
}

CustomEdit::CustomEdit(TextParamsWindow* textParamsWindow)
{
    textParamsWindow_ = textParamsWindow;
}

CustomEdit::~CustomEdit()
{

}

LRESULT CustomEdit::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if ( wParam == VK_RETURN ) {
        textParamsWindow_->NotifyParent(CFM_SIZE);
        bHandled = true;
        return 1;
    }    
    return 0;
}

LRESULT CustomEdit::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = false;
    if ( wParam == VK_RETURN ) {
        bHandled = true; // stop annoying "ding" sound
        return 1;
    }    
    return 0;
}
