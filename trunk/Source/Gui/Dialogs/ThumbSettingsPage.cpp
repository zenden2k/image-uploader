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
#include "atlheaders.h"
#include <gdiplus.h>
#include <GdiPlusPixelFormats.h>
#include "ThumbSettingsPage.h"

#include <uxtheme.h>
#include "LogWindow.h"
#include "Func/LangClass.h"
#include "Func/Settings.h"
#include "Gui/GuiTools.h"
#include "Core/Images/Thumbnail.h"
#include "Func/MyUtils.h"
#include "ThumbEditor.h"
#include "InputDialog.h"
#include <Func/IuCommonFunctions.h>
#include <Func/WinUtils.h>

#pragma comment( lib, "uxtheme.lib" )
// CThumbSettingsPage
CThumbSettingsPage::CThumbSettingsPage()
{
	params_ = Settings.ThumbSettings;
   m_CatchFormChanges = false;
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

	TRC(IDC_TEXTONIMAGEGROUP, "Текст на картинке");
	TRC(IDC_ENTERYOURTEXTLABEL, "Введите текст:");
	TRC(IDC_THUMBFONT, "Шрифт...");
	TRC(IDC_THUMBTEXTCHECKBOX, "Надпись на миниатюре:");
	TRC(IDC_THUMBBACKGROUNDLABEL, "Цвет фона:");
	TRC(IDC_WIDTHRADIO, "Ширина:");
	TRC(IDC_HEIGHTRADIO, "Высота:");
	TRC(IDC_THUMBSCOMBOLABEL, "Шаблон миниатюры:");
	TRC(IDC_EDITTHUMBNAILPRESET, "Редактировать");
	TRC(IDC_NEWTHUMBNAIL, "Создать копию");
	TRC(IDC_THUMBFORMATLABEL,"Формат:");
	TRC(IDC_THUMBQUALITYLABEL,"Качество:");
	
	ThumbBackground.SubclassWindow(GetDlgItem(IDC_THUMBBACKGROUND));
	RECT rc = {13, 170, 290, 400};
	img.SubclassWindow(GetDlgItem(IDC_COMBOPREVIEW));
	//img.Create(m_hWnd, rc);
	img.LoadImage(0);

	SendDlgItemMessage(IDC_THUMBQUALITYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );	
	SetDlgItemText(IDC_THUMBTEXT, Settings.ThumbSettings.Text);

	GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBFORMATLIST, 4, TR("Как у изображения"),
		_T("JPEG"), _T("PNG"), _T("GIF"));

	std::vector<CString> files;
	CString folder = IuCommonFunctions::GetDataFolder()+_T("\\Thumbnails\\");
	WinUtils::GetFolderFileList(files, folder , _T("*.xml"));
	for(size_t i=0; i<files.size(); i++)
		GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBSCOMBO, 1, Utf8ToWCstring(IuCoreUtils::ExtractFileNameNoExt( WCstringToUtf8( files[i]))) );
	
	
	SendDlgItemMessage(IDC_THUMBTEXTCHECKBOX, BM_SETCHECK, Settings.ThumbSettings.ThumbAddImageSize);
	SendDlgItemMessage(IDC_THUMBSCOMBO, CB_SELECTSTRING, -1, (LPARAM) (LPCTSTR) Settings.ThumbSettings.FileName);
	SetDlgItemText(IDC_THUMBTEXT, params_.Text);
	SetDlgItemInt(IDC_THUMBQUALITYEDIT, Settings.ThumbSettings.Quality);
	SendDlgItemMessage(IDC_THUMBFORMATLIST, CB_SETCURSEL, Settings.ThumbSettings.Format);
    SendDlgItemMessage(IDC_WIDTHRADIO, BM_SETCHECK,  !Settings.ThumbSettings.ScaleByHeight);
	 SendDlgItemMessage(IDC_HEIGHTRADIO, BM_SETCHECK,  Settings.ThumbSettings.ScaleByHeight);
	SetDlgItemInt(IDC_WIDTHEDIT,Settings.ThumbSettings.ThumbWidth);
   SetDlgItemInt(IDC_HEIGHTEDIT,Settings.ThumbSettings.ThumbHeight);
    BOOL b;
	ThumbBackground.SetColor(Settings.ThumbSettings.BackgroundColor);
	OnThumbComboChanged(0,0,0,b);
    ::EnableWindow(GetDlgItem(IDC_WIDTHEDIT), !params_.ScaleByHeight);
   ::EnableWindow(GetDlgItem(IDC_HEIGHTEDIT), params_.ScaleByHeight);
 m_CatchFormChanges = true;
	return 1;  // Let the system set the focus
}

bool CThumbSettingsPage::Apply()
{
	Settings.ThumbSettings.ThumbAddImageSize =  SendDlgItemMessage(IDC_THUMBTEXTCHECKBOX, BM_GETCHECK) == BST_CHECKED;
	TCHAR buf[256] =_T("\0");
	GetDlgItemText(IDC_THUMBSCOMBO, buf, 255);
 	Settings.ThumbSettings.FileName =buf;
	Settings.ThumbSettings.Format = static_cast<ThumbCreatingParams::ThumbFormatEnum>(SendDlgItemMessage(IDC_THUMBFORMATLIST, CB_GETCURSEL ));
	Settings.ThumbSettings.Quality = GetDlgItemInt(IDC_THUMBQUALITYEDIT);
   Settings.ThumbSettings.ScaleByHeight = SendDlgItemMessage(IDC_WIDTHRADIO, BM_GETCHECK) == FALSE;
	Settings.ThumbSettings.Text = GuiTools::GetWindowText(GetDlgItem(IDC_THUMBTEXT));
	Settings.ThumbSettings.ThumbWidth = GetDlgItemInt(IDC_WIDTHEDIT);
   Settings.ThumbSettings.ThumbHeight = GetDlgItemInt(IDC_HEIGHTEDIT);
	Settings.ThumbSettings.BackgroundColor = ThumbBackground.GetColor();
   for(std::map<std::string, Thumbnail*>::const_iterator it = thumb_cache_.begin(); it!= thumb_cache_.end(); ++it)
	{
		it->second->SaveToFile();
	}
	return TRUE;
}

LRESULT  CThumbSettingsPage::OnBnClickedNewThumbnail(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::string fileName = getSelectedThumbnailFileName();
	if(fileName.empty())
		return 0;
	std::string newName = "copy_" + IuCoreUtils::ExtractFileNameNoExt(fileName);
	CInputDialog dlg(TR("Окно ввода"), TR("Введите имя нового шаблона миниатюры:"), Utf8ToWCstring(newName));
	if(dlg.DoModal() == IDOK)
	{
		newName = WCstringToUtf8(dlg.getValue());
	}
	else return 0;
	std::string destination = IuCoreUtils::ExtractFilePath(fileName) +"/" + newName + ".xml";
	if(IuCoreUtils::FileExists(destination))
	{
		MessageBox(TR("Шаблон с таким именем уже существует!"), APPNAME, MB_ICONERROR);
		return 0;
	}
	if(IuCoreUtils::copyFile(fileName, destination))
	{
		GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBSCOMBO, 1, Utf8ToWCstring(newName) );
		Thumbnail * thumb = 0;
		if(thumb_cache_.count(fileName))
		{
			thumb  = thumb_cache_[fileName];
			thumb->SaveToFile(destination);
		}
	}
	SendDlgItemMessage(IDC_THUMBSCOMBO, CB_SELECTSTRING, -1, (LPARAM) (LPCTSTR)Utf8ToWCstring( newName));
	showSelectedThumbnailPreview();
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
	CString folder = IuCommonFunctions::GetDataFolder()+_T("\\Thumbnails\\");
	return WCstringToUtf8(folder + thumbFileName+".xml");
}

void CThumbSettingsPage::showSelectedThumbnailPreview()
{
	using namespace Gdiplus;
	std::string fileName = getSelectedThumbnailFileName();
	if(fileName.empty())
		return ;
	CString folder = IuCommonFunctions::GetDataFolder()+_T("\\Thumbnails\\");
	
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
		MessageBox(TR("Не могу загрузить файл миниатюры!"));
		return ;
	}
	
	Bitmap *toUse = bm->Clone(0,300, bm->GetWidth(), bm->GetHeight()-300, PixelFormatDontCare);
	
	Image *result = 0;
	conv.createThumbnail(toUse, &result, 50*1024);
	if(result)	
	img.LoadImage(0, result);

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
   if(!m_CatchFormChanges) return 0;
	params_.Text = GuiTools::GetWindowText(GetDlgItem(IDC_THUMBTEXT));
	showSelectedThumbnailPreview();
	return 0;
}

LRESULT CThumbSettingsPage::OnWidthEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
   if(!m_CatchFormChanges) return 0;
   params_.ScaleByHeight = SendDlgItemMessage(IDC_WIDTHRADIO, BM_GETCHECK) == FALSE;
   ::EnableWindow(GetDlgItem(IDC_WIDTHEDIT), !params_.ScaleByHeight);
   ::EnableWindow(GetDlgItem(IDC_HEIGHTEDIT), params_.ScaleByHeight);
	params_.ThumbWidth = GetDlgItemInt(IDC_WIDTHEDIT);
	params_.ThumbHeight = GetDlgItemInt(IDC_HEIGHTEDIT);
   showSelectedThumbnailPreview();
   return 0;
}
