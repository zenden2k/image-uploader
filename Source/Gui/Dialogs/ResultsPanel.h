/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Func/common.h"
#include "Core/Upload/UploadEngine.h"
#include "Gui/WizardCommon.h"
#include "3rdpart/thread.h"

#define IDC_OPTIONSMENU 10002
#define IDC_USEDIRECTLINKS 10003
#define IDC_SHORTENURLITEM 10004
#define IDC_PREVIEWBUTTON 1006
#define IDC_OPENLINKSINNEWTAB 1007

#define IDC_COPYFOLDERURL 10040


#define IDC_RESULTSTOOLBAR 5000
class CResultsPanel;

// CResultsPanel
class CWizardDlg;
struct IU_Result_Template
{
    CString Name,Items,LineSep,LineStart,ItemSep,LineEnd,TemplateText;
};
class CWebViewWindow;

class CResultsPanel : 
    public CDialogImpl<CResultsPanel>    
{
    public:
        CResultsPanel(CWizardDlg *dlg, std::vector<CUrlListItem>  & urlList, bool openedFromHistory = false);
        virtual ~CResultsPanel();
        enum { IDD = IDD_RESULTSPANEL};

        BEGIN_MSG_MAP(CResultsPanel)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDC_THUMBSPERLINE,EN_CHANGE,OnEditChanged)
            COMMAND_HANDLER(IDC_CODETYPE, CBN_SELCHANGE, OnCbnSelchangeCodetype)
            COMMAND_HANDLER(IDC_COPYALL, BN_CLICKED, OnBnClickedCopyall)
            COMMAND_HANDLER(IDC_USEDIRECTLINKS, BN_CLICKED, OnUseDirectLinksClicked)
            COMMAND_HANDLER(IDC_USETEMPLATE, BN_CLICKED, OnUseTemplateClicked)
            COMMAND_HANDLER(IDC_SHORTENURLITEM, BN_CLICKED, OnShortenUrlClicked)
            COMMAND_HANDLER(IDC_PREVIEWBUTTON, BN_CLICKED, OnPreviewButtonClicked)
            
            //COMMAND_HANDLER(, BN_CLICKED, OnCopyFolderUrlClicked)
            COMMAND_RANGE_HANDLER(IDC_COPYFOLDERURL, IDC_COPYFOLDERURL + 1000, OnCopyFolderUrlClicked);
            COMMAND_HANDLER(IDC_MEDIAFILEINFO, BN_CLICKED, OnBnClickedMediaInfo)
            COMMAND_HANDLER(IDC_VIEWLOG, BN_CLICKED, OnBnClickedViewLog)
            NOTIFY_HANDLER(IDC_RESULTSTOOLBAR, TBN_DROPDOWN, OnOptionsDropDown);
        NOTIFY_HANDLER_EX(IDC_RESULTSTOOLBAR, NM_CUSTOMDRAW, OnResulttoolbarNMCustomDraw)
        END_MSG_MAP()

    fastdelegate::FastDelegate1<bool> OnShortenUrlChanged;
    enum TabPage { kBbCode = 0, kHtml, kPlainText };

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnOptionsDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnUseTemplateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnUseDirectLinksClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCopyFolderUrlClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnPreviewButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
     
    
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    CToolBarCtrl Toolbar;
    void SetPage(TabPage Index);
    void setEngineList(CMyEngineList* EngineList);
    std::vector<CUrlListItem>  &UrlList;
    const CString GenerateOutput();
    CWebViewWindow* webViewWindow_;
    
    bool LoadTemplate();
    LPTSTR TemplateHead,TemplateFoot; //TemplateFoot is only pointer to part of TemplateHead 
  
    TabPage m_Page;
    LRESULT OnCbnSelchangeCodetype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEditChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnShortenUrlClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    
    int GetCodeType();
    void UpdateOutput();
    void SetCodeType(int Index);
    void Clear();
    void EnableMediaInfo(bool Enable);
    CWizardDlg *WizardDlg;
    CMyEngineList *m_EngineList;
    CAtlArray<IU_Result_Template> Templates;
    bool LoadTemplates(CString &Error);
    bool LoadTemplateFromFile(const CString fileName, CString &Error);
    std::map<CString, CString> m_Vars;
    std::vector<ServerProfile> m_Servers;
    CString ReplaceVars(const CString& Text);
    CAutoCriticalSection UrlListCS;
    int m_nImgServer, m_nFileServer;
    void AddServer(ServerProfile server);
    RECT rectNeeded;
    bool shortenUrl_;
    bool openedFromHistory_;
    void InitUpload();
    void setUrlList(CAtlArray<CUrlListItem>  * urlList);
    LRESULT OnResulttoolbarNMCustomDraw(LPNMHDR pnmh);
    void BBCode_Link(CString &Buffer, CUrlListItem &item);
    void HTML_Link(CString &Buffer, CUrlListItem &item);
};


