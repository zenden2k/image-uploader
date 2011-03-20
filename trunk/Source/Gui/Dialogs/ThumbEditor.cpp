/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ThumbEditor.h"
#include "../../LangClass.h"
#include "../../Settings.h"
// CThumbEditor
CThumbEditor::CThumbEditor(Thumbnail *thumb)
{
	thumb_ = thumb;
}

CThumbEditor::~CThumbEditor()
{
}

LRESULT CThumbEditor::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRC(IDCANCEL, "Отмена");
	TRC(IDOK, "OK");
	TRC(IDC_THUMBSTROKECOLOR, "Цвет каймы:");
	SetWindowText(TR("Редактирование шаблона миниатюры"));
	FrameColor.SubclassWindow(GetDlgItem(IDC_FRAMECOLOR));
	Color1.SubclassWindow(GetDlgItem(IDC_COLOR1));
	Color2.SubclassWindow(GetDlgItem(IDC_COLOR2));
	
	ThumbTextColor.SubclassWindow(GetDlgItem(IDC_THUMBTEXTCOLOR));
	CenterWindow(GetParent());

	LoadParams();
	ShowTextCheckboxChanged();
	DrawFrameCheckboxChanged();
	return 0;  // Let the system set the focus
}

LRESULT CThumbEditor::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	/*Settings.ThumbSettings.FrameColor = FrameColor.GetColor();	
	Settings.ThumbSettings.ThumbColor1 = Color1.GetColor();
	Settings.ThumbSettings.ThumbColor2 = Color2.GetColor();	
	Settings.ThumbSettings.ThumbTextColor=ThumbTextColor.GetColor();
	Settings.ImageSettings.TextColor=TextColor.GetColor();
	Settings.ImageSettings.StrokeColor=StrokeColor.GetColor();*/

	SaveParams();
	Settings.ThumbSettings.TextOverThumb = SendDlgItemMessage(IDC_TEXTOVERTHUMB2, BM_GETCHECK);
	EndDialog(wID);
	return 0;
}

LRESULT CThumbEditor::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

COLORREF RGB2COLORREF(unsigned int color)
{
	return RGB(GetBValue(color), GetGValue(color), GetRValue(color));
}

unsigned int COLORREF2RGB( COLORREF color)
{
	return RGB(GetBValue(color), GetGValue(color), GetRValue(color));
}
void CThumbEditor::LoadParams()
{
	bool DrawFrame = thumb_->getParam("DrawFrame")!=0;
	SendDlgItemMessage(IDC_DRAWFRAME, BM_SETCHECK, DrawFrame);

	bool DrawText = thumb_->getParam("DrawText")!=0;
	SendDlgItemMessage(IDC_ADDFILESIZE, BM_SETCHECK, DrawText);

	SetDlgItemInt(IDC_FRAMEWIDTH, thumb_->getParam("FrameWidth"));
	FrameColor.SetColor(RGB2COLORREF(thumb_->getColor("FrameColor")));
	Color1.SetColor(RGB2COLORREF(thumb_->getColor("GradientColor1")));
	ThumbTextColor.SetColor(RGB2COLORREF(thumb_->getColor("TextColor")));
	Color2.SetColor(RGB2COLORREF(thumb_->getColor("GradientColor2")));
	SetDlgItemText(IDC_THUMBTEXT, Utf8ToWCstring(thumb_->getParamString("Text")));
}

void CThumbEditor::SaveParams()
{
	thumb_->setColor("FrameColor", COLORREF2RGB(FrameColor.GetColor()));
	thumb_->setColor("GradientColor1", COLORREF2RGB(Color1.GetColor()));
	thumb_->setColor("TextColor", COLORREF2RGB(ThumbTextColor.GetColor()));
	thumb_->setColor("GradientColor2", COLORREF2RGB(Color2.GetColor()));
	bool DrawFrame = SendDlgItemMessage(IDC_DRAWFRAME, BM_GETCHECK)!=0;
	thumb_->setParam("DrawFrame", DrawFrame);
	thumb_->setParam("FrameWidth", GetDlgItemInt(IDC_FRAMEWIDTH));
	CString text  = IU_GetWindowText(GetDlgItem(IDC_THUMBTEXT));
	bool AddText  = SendDlgItemMessage(IDC_ADDFILESIZE, BM_GETCHECK)!=0;
	thumb_->setParamString("Text", WCstringToUtf8(text));
	thumb_->setParam("DrawText", AddText);
}

LRESULT  CThumbEditor::OnShowTextCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ShowTextCheckboxChanged();
	return 0;
}
		
void CThumbEditor::ShowTextCheckboxChanged()
{
	bool bChecked = SendDlgItemMessage(IDC_ADDFILESIZE, BM_GETCHECK)!=0;
	EnableNextN(GetDlgItem(IDC_ADDFILESIZE), 7, bChecked);
}

LRESULT CThumbEditor::OnDrawFrameCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DrawFrameCheckboxChanged();
	return 0;
}
		
void CThumbEditor::DrawFrameCheckboxChanged()
{
	bool bChecked = SendDlgItemMessage(IDC_DRAWFRAME, BM_GETCHECK)!=0;
	EnableNextN(GetDlgItem(IDC_DRAWFRAME), 3, bChecked);
}