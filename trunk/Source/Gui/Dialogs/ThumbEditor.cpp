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
#include "../../atlheaders.h"
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
	TRC(IDC_COLORSGROUP, "Цвета");
	TRC(IDC_FRAMECOLORLABEL, "Цвет рамки:");
	TRC(IDC_GRADIENTCOLOR1LABEL, "Цвет градиента 1:");
	TRC(IDC_GRADIENTCOLOR2LABEL, "Цвет градиента 2:");
	TRC(IDC_DRAWFRAME, "Обводить в рамку");
	TRC(IDC_FRAMEWIDTHLABEL, "Толщина:");
	TRC(IDC_THUMBPARAMS, "Параметры текста:");
	TRC(IDC_THUMBTEXTCOLORLABEL, "Цвет текста:");
	TRC(IDC_THUMBSTROKECOLORLABEL, "Цвет обводки:");
	TRC(IDC_THUMBTEXTLABEL, "Текст:");
	TRC(IDC_THUMBFONT, "Шрифт...");
	TRC(IDC_ADDFILESIZE, "Выводить текст");
	
	SetWindowText(TR("Редактирование шаблона миниатюры"));
	FrameColor.SubclassWindow(GetDlgItem(IDC_FRAMECOLOR));
	Color1.SubclassWindow(GetDlgItem(IDC_COLOR1));
	Color2.SubclassWindow(GetDlgItem(IDC_COLOR2));
	StrokeColor.SubclassWindow(GetDlgItem(IDC_THUMBTEXTCOLOR2));
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


void CThumbEditor::LoadParams()
{
	StringToFont(_T("Tahoma,7,b,204"), &ThumbFont);
	if(thumb_->existsParam("Font"))
	{
		std::string font = thumb_->getParamString("Font");
		CString wide_text = Utf8ToWCstring(font);
		StringToFont(wide_text, &ThumbFont);
	}

	if(thumb_->existsParam("FrameWidth"))
		SetDlgItemInt(IDC_FRAMEWIDTH, thumb_->getParam("FrameWidth"));
	else 
		::EnableWindow(GetDlgItem(IDC_FRAMEWIDTH), false);

	if(thumb_->existsParam("DrawFrame"))
	{
		bool DrawFrame = thumb_->getParam("DrawFrame")!=0;
		SendDlgItemMessage(IDC_DRAWFRAME, BM_SETCHECK, DrawFrame);
	}
	else
	{
		EnableNextN(GetDlgItem(IDC_DRAWFRAME), 3, false);
		::EnableWindow(GetDlgItem(IDC_DRAWFRAME), false);
	}

	bool DrawText = thumb_->getParam("DrawText")!=0;
	SendDlgItemMessage(IDC_ADDFILESIZE, BM_SETCHECK, DrawText);

	

	if(thumb_->existsParam("FrameColor"))
		FrameColor.SetColor(RGB2COLORREF(thumb_->getColor("FrameColor")));
	else 
	{
		FrameColor.EnableWindow(false);
		::EnableWindow(GetDlgItem(IDC_FRAMECOLORLABEL), false);
	}

	if(thumb_->existsParam("GradientColor1"))
		Color1.SetColor(RGB2COLORREF(thumb_->getColor("GradientColor1")));
	else
	{
		Color1.EnableWindow(false);
		::EnableWindow(GetDlgItem(IDC_GRADIENTCOLOR1LABEL), false);
	}

	if(thumb_->existsParam("TextColor"))
	{
		ThumbTextColor.SetColor(RGB2COLORREF(thumb_->getColor("TextColor")));
	}
	else
	{
		ThumbTextColor.EnableWindow(false);
		::EnableWindow(GetDlgItem(IDC_THUMBTEXTCOLORLABEL), false);
	}

	if(thumb_->existsParam("StrokeColor"))
	{
		StrokeColor.SetColor(RGB2COLORREF(thumb_->getColor("StrokeColor")));
	}
	else
	{
		StrokeColor.EnableWindow(false);
		::EnableWindow(GetDlgItem(IDC_THUMBSTROKECOLORLABEL), false);
	}


	if(thumb_->existsParam("GradientColor2"))
	Color2.SetColor(RGB2COLORREF(thumb_->getColor("GradientColor2")));
	else
	{
		Color2.EnableWindow(false);
		::EnableWindow(GetDlgItem(IDC_GRADIENTCOLOR2LABEL), false);
	}
	SetDlgItemText(IDC_THUMBTEXT, Utf8ToWCstring(thumb_->getParamString("Text")));
}

void CThumbEditor::SaveParams()
{
	if(FrameColor.IsWindowEnabled())
		thumb_->setColor("FrameColor", COLORREF2RGB(FrameColor.GetColor()));
	if(Color1.IsWindowEnabled())
		thumb_->setColor("GradientColor1", COLORREF2RGB(Color1.GetColor()));
	if(ThumbTextColor.IsWindowEnabled())
		thumb_->setColor("TextColor", COLORREF2RGB(ThumbTextColor.GetColor()));

	if(StrokeColor.IsWindowEnabled())
		thumb_->setColor("StrokeColor", COLORREF2RGB(StrokeColor.GetColor()));
	
	if(Color2.IsWindowEnabled())
	thumb_->setColor("GradientColor2", COLORREF2RGB(Color2.GetColor()));
	
	if(thumb_->existsParam("DrawFrame"))
	{
		bool DrawFrame = SendDlgItemMessage(IDC_DRAWFRAME, BM_GETCHECK)!=0;
		thumb_->setParam("DrawFrame", DrawFrame);
	}
	if(thumb_->existsParam("FrameWidth"))
		thumb_->setParam("FrameWidth", GetDlgItemInt(IDC_FRAMEWIDTH));
	
	CString text  = IU_GetWindowText(GetDlgItem(IDC_THUMBTEXT));
	bool AddText  = SendDlgItemMessage(IDC_ADDFILESIZE, BM_GETCHECK)!=0;
	
	thumb_->setParamString("Text", WCstringToUtf8(text));
	if(thumb_->existsParam("DrawText"))
		thumb_->setParam("DrawText", AddText);

	//if(thumb_->existsParam("Font"))
	{
		CString res;
		FontToString(&ThumbFont, res);
		thumb_->setParamString("Font", WCstringToUtf8(res));
	}

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


LRESULT CThumbEditor::OnFontSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// Font selection dialog
	CFontDialog dlg(&ThumbFont);
	dlg.DoModal(m_hWnd);
	return 0;
}