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

#include "LogoSettings.h"
#include "atlheaders.h"
#include <uxtheme.h>
#include "LogWindow.h"
#include "Func/Settings.h"
#include "Gui/GuiTools.h"
#include "InputDialog.h"
#include <Func/WinUtils.h>
#include <Func/Common.h>

// CLogoSettings
CLogoSettings::CLogoSettings()
{
	ZeroMemory(&lf, sizeof(lf));
   m_CatchChanges = false;
   m_ProfileChanged = false;
}

void CLogoSettings::TranslateUI()
{
	TRC(IDC_CURRENTPROFILELABEL, "������� �������:");
	TRC(IDC_FORMATLABEL,"������:");
	TRC(IDC_QUALITYLABEL,"��������:");
	TRC(IDC_RESIZEBYWIDTH,"��������� ������:");
	TRC(IDC_YOURLOGO,"�������� ������� ����");
	TRC(IDC_SMARTCONVERTING, "�� ������������ �����������, ���� ��� ��� � ������ �������");
	TRC(IDC_YOURTEXT,"�������� ����� �� ��������");
	TRC(IDC_XLABEL,"�/��� ������:");
	TRC(IDC_RESIZEMODELABEL, "�����:");
	TRC(IDC_FILENAMELABEL, "��� �����:");
	TRC(IDC_LOGOPOSITIONLABEL, "������� ��������:");
	TRC(IDC_LOGOGROUP, "������� ����");
	TRC(IDC_TEXTONIMAGEGROUP, "����� �� ��������");
	TRC(IDC_ENTERYOURTEXTLABEL, "������� �����:");
	TRC(IDC_TEXTCOLORLABEL, "���� ������:");
	TRC(IDC_TEXTSTROKECOLOR, "���� �������:");
	TRC(IDC_SELECTFONT, "�����...");
	TRC(IDC_TEXTPOSITIONLABEL, "������� ������:");
	TRC(IDC_THUMBPARAMS, "��������� �������");
	TRC(IDC_FRAMECOLORLABEL, "���� �����");
	TRC(IDC_THUMBTEXTCOLORLABEL, "���� ������:");
	TRC(IDC_GRADIENTCOLOR1LABEL, "���� ��������� 1:");
	TRC(IDC_GRADIENTCOLOR2LABEL, "���� ��������� 2:");
	TRC(IDC_ENLARGEIMAGECHECKBOX, "��������� ���������� �����������");
	SetWindowText(TR("�������������� ���������"));	
}

CLogoSettings::~CLogoSettings()
{
		
}

LRESULT CLogoSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   �onvert_profiles_ = Settings.ConvertProfiles;
	TabBackgroundFix(m_hWnd);
	// Translating controls
	TranslateUI();
	RECT rc = {13, 20, 290, 95};
	img.Create(GetDlgItem(IDC_LOGOGROUP), rc);
   img.ShowWindow(SW_HIDE);
	img.LoadImage(NULL);

   GuiTools::AddComboBoxItems(m_hWnd, IDC_RESIZEMODECOMBO, 3, TR("���������"), TR("�� ������"), TR("���������"));
   GuiTools::AddComboBoxItems(m_hWnd, IDC_FORMATLIST, 4, TR("����"), _T("JPEG"), _T("PNG"),_T("GIF"));
  
	SendDlgItemMessage(IDC_TRANSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)0) );
	SendDlgItemMessage(IDC_QUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));

	std::map<ImageConvertingParams::LogoPositionEnum, CString> logoPositions;
	logoPositions[ImageConvertingParams::lpTopLeft]      = TR("������� ����� ����");
	logoPositions[ImageConvertingParams::lpTopCenter]    = TR("������ ����������");
	logoPositions[ImageConvertingParams::lpTopRight]     = TR("������ ������� ����");
	logoPositions[ImageConvertingParams::lpBottomLeft]   = TR("����� ������ ����");
	logoPositions[ImageConvertingParams::lpBottomCenter] = TR("����� ����������");
	logoPositions[ImageConvertingParams::lpBottomRight]  = TR("������ ������ ����");

	for( std::map<ImageConvertingParams::LogoPositionEnum, CString>::const_iterator it = logoPositions.begin(); it != logoPositions.end(); ++it ) {
		GuiTools::AddComboBoxItem(m_hWnd, IDC_LOGOPOSITION, it->second);
		GuiTools::AddComboBoxItem(m_hWnd, IDC_TEXTPOSITION, it->second);
	}
	
   TextColor.SubclassWindow(GetDlgItem(IDC_SELECTCOLOR));
   StrokeColor.SubclassWindow(GetDlgItem(IDC_STROKECOLOR));

   CIcon ico = (HICON)LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONWHITEPAGE));
   RECT profileRect;
   ::GetWindowRect(GetDlgItem(IDC_PROFILETOOBLARPLACEBUTTON), &profileRect);
   ::MapWindowPoints(0, m_hWnd, (LPPOINT)&profileRect, 2);

   m_ProfileEditToolbar.Create(m_hWnd,profileRect,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD | TBSTYLE_LIST |TBSTYLE_FLAT| TBSTYLE_TOOLTIPS|CCS_NORESIZE|/*CCS_BOTTOM |CCS_ADJUSTABLE|*/CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
	m_ProfileEditToolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
   m_ProfileEditToolbar.SetButtonStructSize();
   m_ProfileEditToolbar.SetButtonSize(17,17);

   CIcon saveIcon = LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONSAVE));
	CIcon deleteIcon = LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONDELETE));
   CImageList list;
   list.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
   list.AddIcon(ico);
   list.AddIcon(saveIcon);
	list.AddIcon(deleteIcon);
   m_ProfileEditToolbar.SetImageList(list);
   m_ProfileEditToolbar.AddButton(IDC_NEWPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 0, TR("������� �������"), 0);
   m_ProfileEditToolbar.AddButton(IDC_SAVEPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("��������� �������"), 0);
	m_ProfileEditToolbar.AddButton(IDC_DELETEPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE,  TBSTATE_ENABLED, 2, TR("������� �������"), 0);

   ShowParams(Settings.CurrentConvertProfileName);
   UpdateProfileList();
   return 1; 
}

LRESULT CLogoSettings::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CLogoSettings::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CLogoSettings::OnBnClickedLogobrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("�����������"))+ _T(" (jpeg, bmp, png, gif ...)"),
		_T("*.jpg;*.gif;*.png;*.bmp;*.tiff"),
		TR("��� �����"),
		_T("*.*"));

	CFileDialog fd(true, 0, 0, OFN_OVERWRITEPROMPT, Buf, m_hWnd);
	
	CString s;
	s = WinUtils::GetAppFolder();
	fd.m_ofn.lpstrInitialDir = s;
	if ( fd.DoModal() != IDOK || !fd.m_szFileName ) return 0;

	LPTSTR FileName = fd.m_szFileName;
	SetDlgItemText(IDC_LOGOEDIT, FileName);
	img.LoadImage(FileName);
	img.Invalidate();
   ProfileChanged();
	return 0;
}

LRESULT CLogoSettings::OnBnClickedSelectfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Font selection dialog
	CFontDialog dlg(&lf);
	if(dlg.DoModal(m_hWnd) == IDOK)
       ProfileChanged();
	return 0;
}

LRESULT CLogoSettings::OnBnClickedThumbfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Font selection dialog
	CFontDialog dlg(&ThumbFont);
	dlg.DoModal(m_hWnd);

	return 0;
}

bool CLogoSettings::Apply()
{
   CString saveToProfile = CurrentProfileName;
   if(CurrentProfileOriginalName == _T("Default"))
	saveToProfile = CurrentProfileOriginalName;

   if(!SaveParams(�onvert_profiles_[saveToProfile]))
      return false;
   Settings.ConvertProfiles = �onvert_profiles_;
	Settings.CurrentConvertProfileName  = saveToProfile;
	return TRUE;
}

LRESULT CLogoSettings::OnYourLogoCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
   bool bChecked = SendDlgItemMessage(IDC_YOURLOGO, BM_GETCHECK) == BST_CHECKED;
	GuiTools::EnableNextN(hWndCtl, 5, bChecked);
    ProfileChanged();
   return 0;
}

LRESULT CLogoSettings::OnAddTextChecboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	bool bChecked = SendDlgItemMessage(IDC_YOURTEXT, BM_GETCHECK) == BST_CHECKED;
	GuiTools::EnableNextN(hWndCtl, 9, bChecked);
	ProfileChanged();
   return 0;
}

void CLogoSettings::ShowParams(const ImageConvertingParams& params)
{
   m_ProfileChanged = false;
   m_CatchChanges = false;
   SetDlgItemText(IDC_LOGOEDIT, params.LogoFileName);
	
	if ( !params.LogoFileName.IsEmpty() )  {
		img.LoadImage(params.LogoFileName);
	}

	SetDlgItemText(IDC_EDITYOURTEXT,params.Text);
   SendDlgItemMessage(IDC_LOGOPOSITION, CB_SETCURSEL, params.LogoPosition);
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_SETCURSEL, params.TextPosition);
	GuiTools::SetCheck(m_hWnd, IDC_ENLARGEIMAGECHECKBOX, params.AllowImageEnlarging);
	TextColor.SetColor(params.TextColor);
   StrokeColor.SetColor(params.StrokeColor);
   lf = params.Font;
   if(params.Quality)
		SetDlgItemInt(IDC_QUALITYEDIT,params.Quality);
	else
		SetDlgItemText(IDC_QUALITYEDIT,_T(""));
   SendDlgItemMessage(IDC_FORMATLIST,CB_SETCURSEL, params.Format);
   
	SendDlgItemMessage(IDC_RESIZEMODECOMBO,CB_SETCURSEL, (WPARAM)(int)(ImageConvertingParams::ImageResizeMode&)params.ResizeMode);
   
   SendDlgItemMessage(IDC_YOURLOGO,BM_SETCHECK,  params.AddLogo);
   SendDlgItemMessage(IDC_YOURTEXT,BM_SETCHECK,  params.AddText);

   GuiTools::SetCheck(m_hWnd, IDC_SMARTCONVERTING, params.SmartConverting);
   SetDlgItemText(IDC_IMAGEWIDTH,params.strNewWidth);

	SetDlgItemText(IDC_IMAGEHEIGHT,params.strNewHeight);

   OnYourLogoCheckboxClicked(0,0,GetDlgItem(IDC_YOURLOGO));
	OnAddTextChecboxClicked(0,0,GetDlgItem(IDC_YOURTEXT));
   m_CatchChanges = true;
}

bool CLogoSettings::SaveParams(ImageConvertingParams& params)
{
	bool addLogo = SendDlgItemMessage(IDC_YOURLOGO,BM_GETCHECK) == BST_CHECKED;
	bool addText = SendDlgItemMessage(IDC_YOURTEXT,BM_GETCHECK) == BST_CHECKED;
	int LogoPos = SendDlgItemMessage(IDC_LOGOPOSITION, CB_GETCURSEL);
	int TextPos = SendDlgItemMessage(IDC_TEXTPOSITION, CB_GETCURSEL);
	
	if(LogoPos == TextPos && addLogo && addText) // ���� "������� ����" � ������� ���������� � ���� � �� �� ����� �� ��������
	{
		if(MessageBox(TR("�� ������������� ������ ��������� ����� � ������� � ����� ����� �� �����������?"),TR("��������� �����������"),MB_ICONQUESTION|MB_YESNO)!=IDYES)
			return false;
	} 

	params.LogoPosition = LogoPos;
	params.TextPosition = TextPos;
   params.LogoFileName = GuiTools::GetWindowText(GetDlgItem(IDC_LOGOEDIT));
	params.Text = GuiTools::GetWindowText(GetDlgItem(IDC_EDITYOURTEXT));;
	params.Font = lf;
	params.AddLogo = addLogo;
   params.AddText = addText;
	params.AllowImageEnlarging = GuiTools::GetCheck(m_hWnd, IDC_ENLARGEIMAGECHECKBOX);

	GuiTools::GetCheck(m_hWnd, IDC_SMARTCONVERTING, params.SmartConverting);
	params.TextColor=TextColor.GetColor();
	params.StrokeColor=StrokeColor.GetColor();
   params.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	params.Format = SendDlgItemMessage(IDC_FORMATLIST, CB_GETCURSEL);
	params.ResizeMode = static_cast<ImageConvertingParams::ImageResizeMode>(SendDlgItemMessage(IDC_RESIZEMODECOMBO,CB_GETCURSEL));
   params.strNewWidth = GuiTools::GetWindowText( GetDlgItem(IDC_IMAGEWIDTH));
   params.strNewHeight =  GuiTools::GetWindowText( GetDlgItem(IDC_IMAGEHEIGHT));
	return true;
}

 void CLogoSettings::UpdateProfileList()
 {
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_RESETCONTENT);
    std::map<CString, ImageConvertingParams> ::const_iterator it;\
    bool found = false;
    for(it = �onvert_profiles_.begin(); it != �onvert_profiles_.end(); ++it)
    {
      GuiTools::AddComboBoxItem(m_hWnd, IDC_PROFILECOMBO, it->first);
      if(it->first == CurrentProfileName) found = true;
    }
    if(!found) GuiTools::AddComboBoxItem(m_hWnd, IDC_PROFILECOMBO, CurrentProfileName);
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_SELECTSTRING, -1,(LPARAM)(LPCTSTR) CurrentProfileName); 
 }
 LRESULT CLogoSettings::OnSaveProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl)
 {
    CInputDialog dlg(TR("��� ������ �������"), TR("������� ��� ������� ��� ����������:"), CurrentProfileOriginalName);
    if(dlg.DoModal()==IDOK)
    {
         ImageConvertingParams params;
         SaveParams(params);
         �onvert_profiles_[dlg.getValue()] = params;
         CurrentProfileName = dlg.getValue();
         CurrentProfileOriginalName = dlg.getValue();
         m_ProfileChanged = false;
         UpdateProfileList();
    }
    return 0;
 }

LRESULT CLogoSettings::OnProfileComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(m_ProfileChanged)
	{
		if(MessageBox(TR("������� ������� �� ��� ��������. ����������?"), APPNAME, MB_YESNO|MB_ICONWARNING)!= IDYES) 
		{
			SendDlgItemMessage(IDC_PROFILECOMBO, CB_SELECTSTRING, -1,(LPARAM)(LPCTSTR) CurrentProfileName); 
			return 0;
		}
	}
	CString profile = GuiTools::GetWindowText(GetDlgItem(IDC_PROFILECOMBO));
	ShowParams(profile);
	UpdateProfileList();
	return 0;
}

void CLogoSettings::ShowParams(const CString profileName)
{
   if(CurrentProfileName == profileName) return;
	CurrentProfileName = profileName;
	CurrentProfileOriginalName = profileName; 
	ShowParams(�onvert_profiles_[profileName]);
	SendDlgItemMessage(IDC_PROFILECOMBO, CB_SELECTSTRING, -1,(LPARAM)(LPCTSTR) profileName); 
}

LRESULT CLogoSettings::OnProfileEditedCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ProfileChanged();
	return 0;
}
	
void CLogoSettings::ProfileChanged()
{
	if(!m_CatchChanges) return;
	if(!m_ProfileChanged)
	{
		CurrentProfileOriginalName = CurrentProfileName;
		CurrentProfileName.Replace(CString(_T(" "))+TR("(�������)"), _T(""));
		CurrentProfileName = CurrentProfileName + _T(" ")+ TR("(�������)");
		m_ProfileChanged = true;
		UpdateProfileList();
	}
}

LRESULT CLogoSettings::OnProfileEditedNotification(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   ProfileChanged();
   return 0;
}

LRESULT CLogoSettings::OnNewProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	CString name = TR("����� �������");
	CString generatedName = name;
	int i = 1;
	while(�onvert_profiles_.count(generatedName) > 0)
	{
		generatedName = name  + _T(" ") + WinUtils::IntToStr(i++);
	}

	//CurrentProfileName = CurrentProfileOriginalName = generatedName;
	m_ProfileChanged = true;
	ShowParams(generatedName);
	UpdateProfileList();
	return 0;
}

LRESULT CLogoSettings::OnDeleteProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	CString question;
	question.Format(TR("�� ������������� ������ ������� ������� '%s'?"), CurrentProfileName);
	if(MessageBox(question,TR("��������� �����������"),MB_ICONQUESTION|MB_YESNO) != IDYES)
		return 0;
	if(CurrentProfileName=="Default") return 0;
	if(�onvert_profiles_.count(CurrentProfileName)>0)
		�onvert_profiles_.erase(CurrentProfileName);
	
	ShowParams("Default");
	UpdateProfileList();
	return 0;
}