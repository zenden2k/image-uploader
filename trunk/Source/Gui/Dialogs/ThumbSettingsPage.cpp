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


#include "ThumbSettingsPage.h"
#include <uxtheme.h>
#include "../../LogWindow.h"
#include "../../LangClass.h"
#include "../../Settings.h"
#include "../GuiTools.h"
#include "../../Core/Images/Thumbnail.h"
#include "../../MyUtils.h"
#include "ThumbEditor.h"
#pragma comment( lib, "uxtheme.lib" )
// CThumbSettingsPage
CThumbSettingsPage::CThumbSettingsPage()
{
	ZeroMemory(&lf, sizeof(lf));
	params_ = Settings.ThumbSettings;
}

CThumbSettingsPage::~CThumbSettingsPage()
{
	for(std::map<std::string, Thumbnail*>::const_iterator it = thumb_cache_.begin(); it!= thumb_cache_.end(); ++it)
	{
		delete it->second;
	}
}

LRESULT CThumbSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TabBackgroundFix(m_hWnd);
	// Translating controls
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_FILENAMELABEL, "Имя файла:");
	TRC(IDC_LOGOPOSITIONLABEL, "Позиция логотипа:");
	TRC(IDC_LOGOGROUP, "Водяной знак");
	TRC(IDC_TEXTONIMAGEGROUP, "Текст на картинке");
	TRC(IDC_ENTERYOURTEXTLABEL, "Введите текст:");
	TRC(IDC_TEXTCOLORLABEL, "Цвет текста:");
	TRC(IDC_TEXTSTROKECOLOR, "Цвет обводки:");
	TRC(IDC_SELECTFONT, "Шрифт...");
	TRC(IDC_TEXTPOSITIONLABEL, "Позиция логотипа:");
	TRC(IDC_THUMBPARAMS, "Параметры эскизов");
	TRC(IDC_FRAMECOLORLABEL, "Цвет рамки");
	TRC(IDC_THUMBTEXTCOLORLABEL, "Цвет текста:");
	TRC(IDC_GRADIENTCOLOR1LABEL, "Цвет градиента 1:");
	TRC(IDC_GRADIENTCOLOR2LABEL, "Цвет градиента 2:");
	TRC(IDC_THUMBFONT, "Шрифт...");
	TRC(IDC_THUMBTEXTLABEL, "Надпись на эскизе:");
	TRC(IDC_TEXTOVERTHUMB2, "Надпись на градиентном фоне эскиза");
	SetWindowText(TR("Дополнительные параметры"));	

	
	RECT rc = {13, 170, 290, 400};
	img.SubclassWindow(GetDlgItem(IDC_COMBOPREVIEW));
	//img.Create(m_hWnd, rc);
	img.LoadImage(0);

	SendDlgItemMessage(IDC_TRANSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)0) );
	
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Верхний левый угол"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Сверху посередине"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Правый верхний угол"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Левый нижний угол"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Снизу посередине"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Правый нижний угол"));

	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Верхний левый угол"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Сверху посередине"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Правый верхний угол"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Левый нижний угол"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Снизу посередине"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("Правый нижний угол"));

	SendDlgItemMessage(IDC_LOGOPOSITION, CB_SETCURSEL, Settings.ImageSettings.LogoPosition);
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_SETCURSEL, Settings.ImageSettings.TextPosition);
	SetDlgItemText(IDC_LOGOEDIT, Settings.ImageSettings.LogoFileName);
	
	if(*Settings.ImageSettings.LogoFileName) 
		img.LoadImage(Settings.ImageSettings.LogoFileName);

	SetDlgItemText(IDC_EDITYOURTEXT, Settings.ImageSettings.Text);
	
	 SendDlgItemMessage(IDC_THUMBTEXTCHECKBOX, BM_SETCHECK, Settings.ThumbSettings.ThumbAddImageSize);
	
	
	/*TextColor.SubclassWindow(GetDlgItem(IDC_SELECTCOLOR));
	TextColor.SetColor(Settings.ImageSettings.TextColor);
	
	StrokeColor.SubclassWindow(GetDlgItem(IDC_STROKECOLOR));
	StrokeColor.SetColor(Settings.ImageSettings.StrokeColor);*/
	SendDlgItemMessage(IDC_TEXTOVERTHUMB2, BM_SETCHECK, Settings.ThumbSettings.TextOverThumb);

	ZGuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBFORMATLIST, 4, TR("Как у изображения"),
		_T("JPEG"), _T("PNG"), _T("GIF"));

	lf = Settings.ImageSettings.Font;
	ThumbFont = Settings.ThumbSettings.ThumbFont;
	
	std::vector<CString> files;
	CString folder = IU_GetDataFolder()+_T("\\Thumbnails\\");
	GetFolderFileList(files, folder , _T("*.xml"));
	for(size_t i=0; i<files.size(); i++)
		ZGuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBSCOMBO, 1, Utf8ToWCstring(IuCoreUtils::ExtractFileNameNoExt( WCstringToUtf8( files[i]))) );
	
	SendDlgItemMessage(IDC_THUMBSCOMBO, CB_SELECTSTRING, -1, (LPARAM) (LPCTSTR) Settings.ThumbSettings.thumbFileName);
	SetDlgItemText(IDC_THUMBTEXT, params_.Text);
	

	SendDlgItemMessage(IDC_THUMBFORMATLIST, CB_SETCURSEL, Settings.ThumbSettings.ThumbFormat);
	BOOL b;
	OnThumbComboChanged(0,0,0,b);
//	ThumbTextCheckboxChange();
	return 1;  // Let the system set the focus
}

LRESULT CThumbSettingsPage::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CThumbSettingsPage::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CThumbSettingsPage::OnBnClickedLogobrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("Изображения"))+ _T(" (jpeg, bmp, png, gif ...)"),
		_T("*.jpg;*.gif;*.png;*.bmp;*.tiff"),
		TR("Все файлы"),
		_T("*.*"));

	CFileDialog fd(true, 0, 0, 4|2, Buf, m_hWnd);
	
	CString s;
	s = GetAppFolder();
	fd.m_ofn.lpstrInitialDir = s;
	if ( fd.DoModal() != IDOK || !fd.m_szFileName ) return 0;

	LPTSTR FileName = fd.m_szFileName;

	SetDlgItemText(IDC_LOGOEDIT, FileName);
	img.LoadImage(FileName);
	img.Invalidate();

	return 0;
}

LRESULT CThumbSettingsPage::OnBnClickedSelectfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Font selection dialog
	CFontDialog dlg(&lf);
	dlg.DoModal(m_hWnd);

	return 0;
}

LRESULT CThumbSettingsPage::OnBnClickedThumbfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Font selection dialog
	CFontDialog dlg(&ThumbFont);
	dlg.DoModal(m_hWnd);
	return 0;
}

bool CThumbSettingsPage::Apply()
{
	Settings.ThumbSettings.ThumbFont = ThumbFont;
	Settings.ThumbSettings.ThumbAddImageSize =  SendDlgItemMessage(IDC_THUMBTEXTCHECKBOX, BM_GETCHECK);
	TCHAR buf[256] =_T("\0");
	GetDlgItemText(IDC_THUMBSCOMBO, buf, 255);
 	Settings.ThumbSettings.thumbFileName =buf;
	Settings.ThumbSettings.ThumbFormat = (ThumbFormatEnum) SendDlgItemMessage(IDC_THUMBFORMATLIST, CB_GETCURSEL );
	
	for(std::map<std::string, Thumbnail*>::const_iterator it = thumb_cache_.begin(); it!= thumb_cache_.end(); ++it)
	{
		it->second->SaveToFile();
	}
	return TRUE;
}

LRESULT  CThumbSettingsPage::OnBnClickedNewThumbnail(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CString folder = IU_GetDataFolder()+_T("\\Thumbnails\\");
	Thumbnail thumb;
	thumb.CreateNew();
	thumb.SaveToFile(WCstringToUtf8(folder + "new.xml"));
	return 0;
}

LRESULT CThumbSettingsPage::OnThumbComboChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	showSelectedThumbnailPreview();
	return 0;
}

LRESULT  CThumbSettingsPage::OnEditThumbnailPreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::string fileName = getSelectedThumbnailFileName();
	std::auto_ptr<Thumbnail> autoPtrThumb;
	Thumbnail * thumb = 0;
	if(thumb_cache_.count(fileName))
	thumb  = thumb_cache_[fileName];
	if(!thumb)
	{ 
		thumb = new Thumbnail();
		autoPtrThumb.reset(thumb);
		if(!thumb->LoadFromFile(fileName))
		{
			MessageBox(TR("Не могу загрузить файл миниатюры!"));
			return 0;
		}
	}
	CThumbEditor dlg(thumb);
	if(dlg.DoModal(m_hWnd) == IDOK)
	{
		thumb_cache_[fileName] = thumb;
		autoPtrThumb.release();
		
		showSelectedThumbnailPreview();
	}
	return 0;
}

std::string CThumbSettingsPage::getSelectedThumbnailFileName()
{
	TCHAR buf[256];
	int index = SendDlgItemMessage(IDC_THUMBSCOMBO, CB_GETCURSEL);
	if(index < 0) return "";
	SendDlgItemMessage(IDC_THUMBSCOMBO, CB_GETLBTEXT, index, (WPARAM)buf);
	CString thumbFileName = buf;
	Thumbnail thumb;
	CString folder = IU_GetDataFolder()+_T("\\Thumbnails\\");
	
	std::string fileName = WCstringToUtf8(folder + thumbFileName+".xml");
	//MessageBoxA(0, fileName.c_str(), 0, 0);
	return fileName;
}

void CThumbSettingsPage::showSelectedThumbnailPreview()
{
	std::string fileName = getSelectedThumbnailFileName();
	if(fileName.empty())
		return ;
	CString folder = IU_GetDataFolder()+_T("\\Thumbnails\\");
	
	std::auto_ptr<Thumbnail> autoPtrThumb;
	Thumbnail * thumb = 0;
	if(thumb_cache_.count(fileName))
		thumb  = thumb_cache_[fileName];
	if(!thumb)
	{ 
		thumb = new Thumbnail();
		autoPtrThumb.reset(thumb);
		if(!thumb->LoadFromFile(fileName))
		{
			WriteLog(logError, _T("CThumbSettingsPage"), TR("Не могу загрузить файл миниатюры!"));
			return;
		}
	}
	CImageConverter conv;
	conv.setThumbCreatingParams(params_);
	conv.setThumbnail(thumb);
	Bitmap * bm = BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_PNG2),_T("PNG"));
	if(!bm) 
	{
		MessageBox(_T("Не могу загрузить из ресурсов результат"));
		return ;
	}
	
	Bitmap *toUse = bm->Clone(0,300, bm->GetWidth(), bm->GetHeight()-300, PixelFormatDontCare);
	
	Image *result = 0;
	conv.createThumbnail(toUse, &result);
	if(result)	
	img.LoadImage(0, result);
	else
		MessageBox(_T("Пустой результат"));

	delete toUse;
	delete result;
	delete bm;
}

LRESULT CThumbSettingsPage::OnThumbTextCheckboxClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ThumbTextCheckboxChange();
	showSelectedThumbnailPreview();
	return 0;
}

void CThumbSettingsPage::ThumbTextCheckboxChange()
{
	bool bChecked = SendDlgItemMessage(IDC_THUMBTEXTCHECKBOX, BM_GETCHECK)==BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_THUMBTEXT), bChecked);
	params_.ThumbAddImageSize = bChecked;
	
}

LRESULT CThumbSettingsPage::OnThumbTextChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	params_.Text = IU_GetWindowText(GetDlgItem(IDC_THUMBTEXT));
	showSelectedThumbnailPreview();
	return 0;
}
	