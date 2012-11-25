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

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Core/Upload/UploadEngine.h"
#include "Gui/WizardCommon.h"
#include "3rdpart/thread.h"
#include <Func/CommonTypes.h>

class CResultsPanel;

// CResultsPanel
class CWizardDlg;
class CMyEngineList;
struct IU_Result_Template
{
	CString Name,Items,LineSep,LineStart,ItemSep,LineEnd,TemplateText;
};

class CResultsPanel : 
	public CDialogImpl<CResultsPanel>	
{
	public:
		CResultsPanel(CWizardDlg *dlg, CAtlArray<CUrlListItem>  & urlList);
		virtual ~CResultsPanel();
	

		enum { IDD = IDD_RESULTSPANEL};

		enum { IDC_OPTIONSMENU = 10002, IDC_USEDIRECTLINKS = 10003,  IDC_COPYFOLDERURL = 10004, IDC_RESULTSTOOLBAR = 5000};

		BEGIN_MSG_MAP(CResultsPanel)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDC_THUMBSPERLINE,EN_CHANGE,OnEditChanged)
			COMMAND_HANDLER(IDC_CODETYPE, CBN_SELCHANGE, OnCbnSelchangeCodetype)
			COMMAND_HANDLER(IDC_COPYALL, BN_CLICKED, OnBnClickedCopyall)
			COMMAND_HANDLER(IDC_USEDIRECTLINKS, BN_CLICKED, OnUseDirectLinksClicked)
			COMMAND_HANDLER(IDC_USETEMPLATE, BN_CLICKED, OnUseTemplateClicked)
			COMMAND_RANGE_HANDLER(IDC_COPYFOLDERURL, IDC_COPYFOLDERURL + 1000, OnCopyFolderUrlClicked);
			
			NOTIFY_HANDLER(IDC_RESULTSTOOLBAR, TBN_DROPDOWN, OnOptionsDropDown);
		NOTIFY_HANDLER_EX(IDC_RESULTSTOOLBAR, NM_CUSTOMDRAW, OnResulttoolbarNMCustomDraw)
		END_MSG_MAP()

	protected:
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnOptionsDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnUseTemplateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnUseDirectLinksClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopyFolderUrlClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	LRESULT OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnResulttoolbarNMCustomDraw(LPNMHDR pnmh);

public:
	LRESULT OnCbnSelchangeCodetype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void SetPage(int Index);
	void setEngineList(CMyEngineList* EngineList);
	const CString GenerateOutput();
	int GetCodeType();
	void UpdateOutput();
	void SetCodeType(int Index);
	void Clear();
	void AddServer(CString server);
	void setUrlList(CAtlArray<CUrlListItem>  * urlList);
	void setRectNeeded(RECT rect);
	void InitUpload();

	CAutoCriticalSection UrlListCS;

private:
	bool LoadTemplate();
	bool LoadTemplates(CString &Error);
	CString ReplaceVars(const CString& Text);


	CToolBarCtrl Toolbar;
	CAtlArray<CUrlListItem>  &UrlList;
	LPTSTR TemplateHead,TemplateFoot; //TemplateFoot is only pointer to part of TemplateHead 
	int m_Page;
	CWizardDlg *WizardDlg;
	CMyEngineList *m_EngineList;
	CAtlArray<IU_Result_Template> Templates;
	std::map<CString, CString> m_Vars;
	std::vector<CString> m_Servers;
	int m_nImgServer, m_nFileServer;
	RECT rectNeeded;
};


