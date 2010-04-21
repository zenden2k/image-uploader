/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
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

#pragma once

#include "resource.h"       // main symbols
#include "common.h"
#define IDC_OPTIONSMENU 10002
#define IDC_USEDIRECTLINKS 10003
#define IDC_COPYFOLDERURL 10004
#define IDC_RESULTSTOOLBAR 5000
//#include "uploaddlg.h"
//#include "wizarddlg.h"
// CResultsPanel
class CWizardDlg;
struct IU_Result_Template
{
	CString Name,Items,LineSep,LineStart,ItemSep,LineEnd,TemplateText;
};

class CResultsPanel : 
	public CDialogImpl<CResultsPanel>	
{
	public:
		CResultsPanel(CAtlArray<CUrlListItem> &p,CWizardDlg *dlg);
		virtual ~CResultsPanel();
		enum { IDD = IDD_RESULTSPANEL};

		BEGIN_MSG_MAP(CResultsPanel)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDC_THUMBSPERLINE,EN_CHANGE,OnEditChanged)
			COMMAND_HANDLER(IDC_CODETYPE, CBN_SELCHANGE, OnCbnSelchangeCodetype)
			COMMAND_HANDLER(IDC_COPYALL, BN_CLICKED, OnBnClickedCopyall)
			COMMAND_HANDLER(IDC_USEDIRECTLINKS, BN_CLICKED, OnUseDirectLinksClicked)
			COMMAND_HANDLER(IDC_USETEMPLATE, BN_CLICKED, OnUseTemplateClicked)
			//COMMAND_HANDLER(, BN_CLICKED, OnCopyFolderUrlClicked)
			
			COMMAND_RANGE_HANDLER(IDC_COPYFOLDERURL, IDC_COPYFOLDERURL + 1000, OnCopyFolderUrlClicked);
		

			COMMAND_HANDLER(IDC_MEDIAFILEINFO, BN_CLICKED, OnBnClickedMediaInfo)
			COMMAND_HANDLER(IDC_VIEWLOG, BN_CLICKED, OnBnClickedViewLog)
			NOTIFY_HANDLER(IDC_RESULTSTOOLBAR, TBN_DROPDOWN, OnOptionsDropDown);
		END_MSG_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnOptionsDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnUseTemplateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnUseDirectLinksClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	 LRESULT OnCopyFolderUrlClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	CToolBarCtrl Toolbar;
	void SetPage(int Index);
	CAtlArray<CUrlListItem> & UrlList;
	void GenerateOutput();
	bool LoadTemplate();
	LPTSTR TemplateHead,TemplateFoot; //TemplateFoot is only pointer to part of TemplateHead 
	int m_Page;
	LRESULT OnCbnSelchangeCodetype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	int GetCodeType();
	void SetCodeType(int Index);
	void Clear();
	void EnableMediaInfo(bool Enable);
	CWizardDlg *WizardDlg;
	CAtlArray<IU_Result_Template> Templates;
	bool LoadTemplates(CString &Error);
	std::map<CString, CString> m_Vars;
	std::map<int,int> m_Servers;
	CString ReplaceVars(const CString Text);
	CAutoCriticalSection UrlListCS;
	int m_nImgServer, m_nFileServer;
	void AddServerId(int serverId);
};


