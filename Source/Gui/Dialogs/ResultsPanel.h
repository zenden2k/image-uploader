/*

    Uptooda - free application for uploading images/files to the Internet

    Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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

#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Func/Common.h"
#include "Core/Upload/UploadEngine.h"
#include "Gui/WizardCommon.h"
#include "Func/MyEngineList.h"
#include "Core/OutputGenerator/AbstractOutputGenerator.h"

#define IDC_OPTIONSMENU 10002
#define IDC_USEDIRECTLINKS 10003
#define IDC_SHORTENURLITEM 10004
#define IDC_PREVIEWBUTTON 1006
#define IDC_OPENLINKSINNEWTAB 1007
#define IDC_GROUPBYFILENAME 1008

#define IDC_COPYFOLDERURL 10040

#define IDC_RESULTSTOOLBAR 5000

class CResultsPanel;

// CResultsPanel
class CWizardDlg;

class CWebViewWindow;

namespace Uptooda::Core::OutputGenerator{
    class XmlTemplateList;
    class XmlTemplateGenerator;
}

class CResultsPanel :
    public CDialogImpl<CResultsPanel>
{
    public:
        CResultsPanel(CWizardDlg *dlg, std::vector<Uptooda::Core::OutputGenerator::UploadObject>& urlList, bool openedFromHistory = false);
        virtual ~CResultsPanel();
        enum { IDD = IDD_RESULTSPANEL};

        BEGIN_MSG_MAP(CResultsPanel)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_TIMER, OnTimer)
            COMMAND_HANDLER(IDC_THUMBSPERLINE,EN_CHANGE,OnEditChanged)
            COMMAND_HANDLER(IDC_CODETYPE, CBN_SELCHANGE, OnCbnSelchangeCodetype)
            COMMAND_HANDLER(IDC_COPYALL, BN_CLICKED, OnBnClickedCopyall)
            COMMAND_HANDLER(IDC_USEDIRECTLINKS, BN_CLICKED, OnUseDirectLinksClicked)
            COMMAND_HANDLER(IDC_USETEMPLATE, BN_CLICKED, OnUseTemplateClicked)
            COMMAND_HANDLER(IDC_SHORTENURLITEM, BN_CLICKED, OnShortenUrlClicked)
            COMMAND_HANDLER(IDC_PREVIEWBUTTON, BN_CLICKED, OnPreviewButtonClicked)
            COMMAND_HANDLER(IDC_GROUPBYFILENAME, BN_CLICKED, OnGroupByFilenameClicked)
            MESSAGE_HANDLER(WM_MY_DPICHANGED, OnMyDpiChanged)
            //COMMAND_HANDLER(, BN_CLICKED, OnCopyFolderUrlClicked)
            COMMAND_RANGE_HANDLER(IDC_COPYFOLDERURL, IDC_COPYFOLDERURL + 1000, OnCopyFolderUrlClicked);

            COMMAND_HANDLER(IDC_MEDIAFILEINFO, BN_CLICKED, OnBnClickedMediaInfo)
            NOTIFY_HANDLER(IDC_RESULTSTOOLBAR, TBN_DROPDOWN, OnOptionsDropDown);
        END_MSG_MAP()
    using ShortenUrlChangedCallback = std::function<void(bool)>;

    using TabPage = Uptooda::Core::OutputGenerator::CodeLang;
    //enum TabPage { kBbCode = 0, kHtml, kPlainText, kMarkdown };
    enum CodeType { ctTableOfThumbnails = 0, ctClickableThumbnails, ctImages, ctLinks };
    enum { kOutputTimer = 1};

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
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMyDpiChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCbnSelchangeCodetype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEditChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnShortenUrlClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnGroupByFilenameClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    void SetPage(TabPage Index);
    void setEngineList(CMyEngineList* EngineList);

    std::string GenerateOutput();
    std::unique_ptr<CWebViewWindow> webViewWindow_;

    /**
     * Returns selected item's index of code type combobox.
     * First four indices correspond to CodeType enum
     */
    int GetCodeType() const;
    void UpdateOutput(bool immediately = false);
    void SetCodeType(int Index);
    void Clear();
    void EnableMediaInfo(bool Enable);
    void InitUpload();
    void AddServer(const ServerProfile& server);
    bool copyResultsToClipboard();
    std::mutex& outputMutex();
    void setRectNeeded(const RECT& rc);
    void setShortenUrls(bool shorten);
    void setOnShortenUrlChanged(ShortenUrlChangedCallback callback);
    void setGroupByFilename(bool enable);
    Uptooda::Core::OutputGenerator::AbstractOutputGenerator* createOrGetGenerator(Uptooda::Core::OutputGenerator::GeneratorID gid, Uptooda::Core::OutputGenerator::CodeLang lang,
        Uptooda::Core::OutputGenerator::CodeType);
protected:
    CToolBarCtrl Toolbar;
    CComboBox codeTypeComboBox;
    TabPage m_Page;
    std::map<std::string, std::string> m_Vars;
    std::vector<ServerProfile> m_Servers;
    std::vector<Uptooda::Core::OutputGenerator::UploadObject>  &UrlList;
    std::mutex urlListMutex_;
    std::unordered_map<Uptooda::Core::OutputGenerator::GeneratorID, std::unique_ptr<Uptooda::Core::OutputGenerator::AbstractOutputGenerator>> outputGenerators_;
    int m_nImgServer, m_nFileServer;
    CWizardDlg *WizardDlg;
    CMyEngineList *m_EngineList;
    std::unique_ptr<Uptooda::Core::OutputGenerator::XmlTemplateList> templateList_;
    std::unique_ptr<Uptooda::Core::OutputGenerator::XmlTemplateGenerator> xmlTemplateGenerator_;
    bool outputChanged_;
    RECT rectNeeded;
    bool shortenUrl_;
    bool openedFromHistory_;
    bool groupByFileName_;
    ShortenUrlChangedCallback onShortenUrlChanged_;
    CImageListManaged toolbarImageList_;
    bool isMediaInfoEnabled_ = false;

    void onCodeTypeChanged();
    void createToolbar();
};


