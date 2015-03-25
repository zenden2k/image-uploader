/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>

    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "TextParamsWindow.h"

#include "resource.h"
#include "Gui/GuiTools.h"
#include <Func/WinUtils.h>
#include <zthread/Thread.h>
#include <Core/3rdpart/FastDelegate.h>
#include <vector>
class FontEnumerator : public ZThread::Runnable {
	
	public:
		typedef fastdelegate::FastDelegate0<void> OnEnumerationFinishedDelegate;
		FontEnumerator(HDC dc, std::vector<LOGFONT>& fonts, const OnEnumerationFinishedDelegate& onEnumerationFinished) : fonts_(fonts) {
			onEnumerationFinished_ = onEnumerationFinished;
			dc_ = dc;
		}

		virtual void run()
		{
			LOGFONT logFont;
			memset(&logFont, 0, sizeof(logFont));
			logFont.lfCharSet = DEFAULT_CHARSET;
			EnumFontFamiliesEx(dc_, &logFont, EnumFontFamExProc, reinterpret_cast<LPARAM>(this),  0);
			std::sort(fonts_.begin(), fonts_.end(), sortCompareLogFont);
			std::vector<LOGFONT>::iterator it;
			it = std::unique(fonts_.begin(), fonts_.end(), compareLogFont);
			fonts_.resize( std::distance(fonts_.begin(),it) );
			onEnumerationFinished_();
		}
	protected:
		OnEnumerationFinishedDelegate onEnumerationFinished_;
		HDC dc_;
		std::vector<LOGFONT>& fonts_;
		

		static int CALLBACK EnumFontFamExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam) {
			FontEnumerator* enumerator = reinterpret_cast<FontEnumerator*>(lParam);
			if ( lpelfe->lfFaceName[0] != _T('@')) {
				enumerator->fonts_.push_back(*lpelfe);
			}
			return 1;
		}

		static bool compareLogFont ( LOGFONT font1, LOGFONT font2)
		{
			return lstrcmp(font1.lfFaceName, font2.lfFaceName) == 0;
		};

		static bool sortCompareLogFont ( LOGFONT font1, LOGFONT font2)
		{
			return lstrcmp(font1.lfFaceName, font2.lfFaceName) <0;
		};


};

TextParamsWindow::TextParamsWindow()
{
	fontEnumerationThread_ = 0;
}

TextParamsWindow::~TextParamsWindow()
{
	if ( fontEnumerationThread_ ) {
		fontEnumerationThread_->wait();
		delete fontEnumerationThread_;
	}
}

void TextParamsWindow::setFont(LOGFONT logFont)
{
	fontName_ = logFont.lfFaceName;
	fontComboBox_.SelectString(-1, fontName_);
	SetDlgItemInt(IDC_FONTSIZECOMBO, MulDiv(-logFont.lfHeight, 72, GetDeviceCaps(dc_, LOGPIXELSY)));
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

	int fontSelectedIndex = fontComboBox_.GetCurSel();
	if ( fontSelectedIndex !=-1 ) {
		logFont = fonts_[fontSelectedIndex];
		int fontSize = GetDlgItemInt(IDC_FONTSIZECOMBO);
		logFont.lfHeight =  -MulDiv(fontSize, GetDeviceCaps(dc_, LOGPIXELSY), 72);
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
	SetWindowText(TR("Параметры текста"));
	fontComboBox_.Attach(GetDlgItem(IDC_FONTCOMBO));
	fontSizeComboBox_.Attach(GetDlgItem(IDC_FONTSIZECOMBO));
	dc_ = GetDC();
	fontEnumerationThread_ = new ZThread::Thread(new FontEnumerator(dc_, fonts_, FontEnumerator::OnEnumerationFinishedDelegate(this, &TextParamsWindow::OnFontEnumerationFinished)));
	GuiTools::AddComboBoxItems(m_hWnd, IDC_FONTSIZECOMBO, 19, _T("7"), _T("8"), _T("9"), _T("10"),_T("11"),_T("12"), _T("13"),
		_T("14"), _T("15"),_T("16"),_T("18"),_T("20"),_T("22"), _T("24"),  _T("26"),  _T("28"),  _T("36"), _T("48"),_T("72")
		);

	RECT toolbarRect;
	HWND toolbarPlaceholder = GetDlgItem(IDC_TOOLBARPLACEHOLDER);
	::GetWindowRect(toolbarPlaceholder, &toolbarRect);
	ScreenToClient(&toolbarRect);
	int iconWidth =  ::GetSystemMetrics(SM_CXSMICON);
	int iconHeight =  ::GetSystemMetrics(SM_CYSMICON);
	toolbarImageList_.Create(iconWidth, iconHeight, ILC_COLOR32,3,3);
	toolbarImageList_.AddIcon(GuiTools::LoadSmallIcon(IDI_ICONBOLD));
	toolbarImageList_.AddIcon(GuiTools::LoadSmallIcon(IDI_ICONITALIC));
	toolbarImageList_.AddIcon(GuiTools::LoadSmallIcon(IDI_ICONUNDERLINE));
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
	int fontCount = fonts_.size();
	for ( int i = 0; i < fontCount; i++ ) {
		fontComboBox_.AddString(fonts_[i].lfFaceName);
	}
	fontComboBox_.SelectString(-1, fontName_);
}

void TextParamsWindow::NotifyParent(DWORD changeMask)
{
	::SendMessage(GetParent(), TPWM_FONTCHANGED, (WPARAM)m_hWnd, (LPARAM)changeMask);
}
