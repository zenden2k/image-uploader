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


#include "UploadParamsDlg.h"
#include "wizarddlg.h"
#include "Func/Common.h"
#include "Func/Settings.h"
#include "Gui/GuiTools.h"
#include <Func/WinUtils.h>
#include <Func/Myutils.h>

// CUploadParamsDlg
CUploadParamsDlg::CUploadParamsDlg(ImageUploadParams params)
{

	params_ = params;
}

CUploadParamsDlg::~CUploadParamsDlg()
{
	
}

LRESULT CUploadParamsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());

	ThumbBackground_.SubclassWindow(GetDlgItem(IDC_THUMBBACKGROUND));
	ThumbBackground_.SetColor(Settings.ThumbSettings.BackgroundColor);

	SetWindowText(TR("Параметры загрузки"));
	TRC(IDC_ADDFILESIZE, "Текст на эскизе");
	TRC(IDC_USESERVERTHUMBNAILS, "Использовать серверные эскизы");
	TRC(IDC_PROCESSIMAGESCHECKBOX, "Обрабатывать изображения");
	TRC(IDCANCEL, "Отмена");

	//Fill profile combobox
	SendDlgItemMessage(IDC_PROFILECOMBO, CB_RESETCONTENT);
	std::map<CString, ImageConvertingParams> ::const_iterator it;
		bool found = false;
	for(it = Settings.ConvertProfiles.begin(); it != Settings.ConvertProfiles.end(); ++it)
	{
		GuiTools::AddComboBoxItem(m_hWnd, IDC_PROFILECOMBO, it->first);
		//if(it->first == CurrentProfileName) found = true;
	}
	if(!found) {  
		SendDlgItemMessage(IDC_PROFILECOMBO, CB_SETCURSEL, 0,0);

		//GuiTools::AddComboBoxItem(m_hWnd, IDC_PROFILECOMBO, CurrentProfileName);
	} else {
//		SendDlgItemMessage(IDC_PROFILECOMBO, CB_SELECTSTRING, -1,(LPARAM)(LPCTSTR) CurrentProfileName); */
	}

	// Fill thumb profiles
	std::vector<CString> files;
	CString folder = IU_GetDataFolder()+_T("\\Thumbnails\\");
	WinUtils::GetFolderFileList(files, folder , _T("*.xml"));
	for(size_t i=0; i<files.size(); i++) {
		GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBTEMPLATECOMBO, 1, Utf8ToWCstring(IuCoreUtils::ExtractFileNameNoExt( WCstringToUtf8( files[i]))) );
	}
	SendDlgItemMessage(IDC_THUMBTEMPLATECOMBO, CB_SETCURSEL, 0,0);
	GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBFORMATLIST, 4, TR("Как у изображения"),
		_T("JPEG"), _T("PNG"), _T("GIF"));

	GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBRESIZECOMBO, 3, 
		TR("По ширине"), TR("По высоте"), TR("По большей стороне") );

	GuiTools::SetCheck(m_hWnd, IDC_PROCESSIMAGESCHECKBOX, params_.ProcessImages);
	GuiTools::SetCheck(m_hWnd, IDC_CREATETHUMBNAILS, params_.CreateThumbs);
	GuiTools::SetCheck(m_hWnd, IDC_USESERVERTHUMBNAILS, params_.UseServerThumbs);
	GuiTools::SetCheck(m_hWnd, IDC_DEFAULTTHUMBSETTINGSCHECKBOX, params_.UseDefaultThumbSettings);
	GuiTools::SetCheck(m_hWnd, IDC_DEFAULTSETTINGSCHECKBOX, params_.UseDefaultSettings);
	//GuiTools::SetCheck(m_hWnd, IDC_DEFAULTTHUMBSETTINGSCHECKBOX, params_.);

	
	
	SendDlgItemMessage(IDC_THUMBQUALITYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );	


	createThumbnailsCheckboxChanged();
	processImagesChanged();
	defaultSettingsCheckboxChanged();
	::SetFocus(GetDlgItem(IDC_LOGINEDIT));
	return 0;  
}

LRESULT CUploadParamsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	GuiTools::GetCheck(m_hWnd, IDC_PROCESSIMAGESCHECKBOX, params_.ProcessImages);
	GuiTools::GetCheck(m_hWnd, IDC_CREATETHUMBNAILS, params_.CreateThumbs);
	GuiTools::GetCheck(m_hWnd, IDC_USESERVERTHUMBNAILS, params_.UseServerThumbs);
	GuiTools::GetCheck(m_hWnd, IDC_DEFAULTTHUMBSETTINGSCHECKBOX, params_.UseDefaultThumbSettings);
	GuiTools::GetCheck(m_hWnd, IDC_DEFAULTSETTINGSCHECKBOX, params_.UseDefaultSettings);
	EndDialog(wID);
	return 0;
}

LRESULT CUploadParamsDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CUploadParamsDlg::OnClickedCreateThumbnailsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	createThumbnailsCheckboxChanged();
	return 0;
}

void CUploadParamsDlg::createThumbnailsCheckboxChanged() {
	bool isChecked = GuiTools::IsChecked(m_hWnd, IDC_CREATETHUMBNAILS);
	GuiTools::EnableNextN(GetDlgItem(IDC_CREATETHUMBNAILS), isChecked? 4 : 10, isChecked );
	if ( isChecked ) {
		defaultThumbSettingsCheckboxChanged();
		thumbTextCheckboxChanged();
	}
}

void CUploadParamsDlg::processImagesChanged() {
	bool isChecked = GuiTools::IsChecked(m_hWnd, IDC_PROCESSIMAGESCHECKBOX);
		GuiTools::EnableNextN(GetDlgItem(IDC_PROCESSIMAGESCHECKBOX), 2, isChecked );
}

LRESULT CUploadParamsDlg::OnClickedProcessImagesCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	processImagesChanged();
	return 0;
}


void CUploadParamsDlg::defaultSettingsCheckboxChanged() {
	bool isChecked = GuiTools::IsChecked(m_hWnd, IDC_DEFAULTSETTINGSCHECKBOX);
	GuiTools::EnableNextN(GetDlgItem(IDC_DEFAULTSETTINGSCHECKBOX), 17, !isChecked );
	
	if ( !isChecked ) {

		createThumbnailsCheckboxChanged();
		processImagesChanged();
	}
}

LRESULT CUploadParamsDlg::OnClickedDefaultSettingsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)  {
	defaultSettingsCheckboxChanged();
	return 0;
}

LRESULT  CUploadParamsDlg::OnClickedDefaultThumbSettingsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	defaultThumbSettingsCheckboxChanged();
	return 0;
}


void  CUploadParamsDlg::defaultThumbSettingsCheckboxChanged() {
	bool useDefaultThumbnailSettings = GuiTools::IsChecked(m_hWnd, IDC_DEFAULTTHUMBSETTINGSCHECKBOX);
	//GuiTools::EnableNextN(GetDlgItem(IDC_DEFAULTTHUMBSETTINGSCHECKBOX), 8, !useDefaultThumbnailSettings );
	
	bool useServerThumbnails = GuiTools::IsChecked(m_hWnd, IDC_USESERVERTHUMBNAILS);

	bool addThumbText = GuiTools::IsChecked(m_hWnd, IDC_THUMBTEXTCHECKBOX);

	std::map<int, bool> enable;
	enable[IDC_THUMBTEMPLATECOMBOLABEL] = !useServerThumbnails && !useDefaultThumbnailSettings;
	enable[IDC_THUMBTEMPLATECOMBO] = !useServerThumbnails && !useDefaultThumbnailSettings;
	enable[IDC_THUMBRESIZECOMBO] = !useServerThumbnails;
	enable[IDC_THUMBTEXTCHECKBOX] = !useDefaultThumbnailSettings;
	enable[IDC_THUMBTEXT] = !useDefaultThumbnailSettings && addThumbText;
	enable[IDC_THUMBRESIZELABEL] = !useDefaultThumbnailSettings;
	enable[IDC_WIDTHEDIT] = !useDefaultThumbnailSettings;
	enable[IDC_WIDTHEDITUNITS] = !useDefaultThumbnailSettings;
	enable[IDC_THUMBRESIZECOMBO] = !useServerThumbnails && !useDefaultThumbnailSettings;
	
	for ( std::map<int,bool>::const_iterator it = enable.begin(); it != enable.end(); ++it) {
		GuiTools::EnableDialogItem(m_hWnd, it->first, it->second);
	}
}

void  CUploadParamsDlg::thumbTextCheckboxChanged() {
	bool isChecked = GuiTools::IsChecked(m_hWnd, IDC_THUMBTEXTCHECKBOX);
	GuiTools::EnableDialogItem(m_hWnd, IDC_THUMBTEXT, isChecked);
}

LRESULT CUploadParamsDlg::OnClickedThumbTextCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	thumbTextCheckboxChanged();
	return 0;
}

LRESULT CUploadParamsDlg::OnClickedUseServerThumbnailsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	useServerThumbnailsChanged();
	return 0;
}

void CUploadParamsDlg::useServerThumbnailsChanged() {
	bool useServerThumbnails = GuiTools::IsChecked(m_hWnd, IDC_USESERVERTHUMBNAILS);

	if ( useServerThumbnails ) {
		// Select item 'resize by width'
		SendDlgItemMessage(IDC_THUMBRESIZECOMBO, CB_SETCURSEL, 0,0);
	}

	defaultThumbSettingsCheckboxChanged();
}

ImageUploadParams CUploadParamsDlg::imageUploadParams() {
	return params_;
}