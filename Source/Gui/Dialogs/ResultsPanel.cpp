/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include "ResultsPanel.h"

#include <utility>

#include "Core/3rdpart/pcreplusplus.h"
#include "UploadSettings.h"
#ifdef IU_ENABLE_MEDIAINFO
#include "MediaInfoDlg.h"
#endif
#include "LogWindow.h"
#include "Gui/Dialogs/WebViewWindow.h"
#include "Core/Utils/TextUtils.h"
#include "Func/IuCommonFunctions.h"
#include "Func/WinUtils.h"
#include "Core/ServiceLocator.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "Core/AppParams.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/GuiTools.h"

// CResultsPanel
CResultsPanel::CResultsPanel(CWizardDlg *dlg, std::vector<CUrlListItem>& urlList, bool openedFromHistory):
    WizardDlg(dlg), UrlList(urlList)
{
    m_nImgServer = m_nFileServer = -1;
    TemplateHead = TemplateFoot = nullptr; 
    openedFromHistory_ = openedFromHistory;
    rectNeeded = {};
    rectNeeded.left = -1;
    CString TemplateLoadError;
    shortenUrl_ = false;
    outputChanged_ = false;
    m_Page = kBbCode;
    m_EngineList = nullptr;
    groupByFileName_ = false;
    if(!LoadTemplates(TemplateLoadError)) {
        ServiceLocator::instance()->logger()->write(ILogger::logWarning, _T("Results Module"), TemplateLoadError);
    }
}

CResultsPanel::~CResultsPanel()
{
    delete[] TemplateHead;
    if (webViewWindow_ ) {
        if (webViewWindow_->m_hWnd) {
            webViewWindow_->DestroyWindow();
        }
    }  
}
    
bool CResultsPanel::LoadTemplate()
{
    if(TemplateHead && TemplateFoot) return true;

    DWORD dwBytesRead;
    CString FileName = IuCommonFunctions::GetDataFolder() + _T("template.txt");

    delete[] TemplateHead;
    TemplateHead = NULL;
    TemplateFoot = NULL;
    HANDLE hFile = CreateFile(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD dwFileSize = GetFileSize(hFile, nullptr);
    if (!dwFileSize) {
        return false;
    }
    DWORD dwMemoryNeeded = std::min(35536ul, dwFileSize);
    LPTSTR TemplateText = (LPTSTR)new CHAR[dwMemoryNeeded+2]; 
    ZeroMemory(TemplateText,dwMemoryNeeded);
    ::ReadFile(hFile, (LPVOID)TemplateText , 2, &dwBytesRead, NULL); //Reading BOM
    if (::ReadFile(hFile, (LPVOID)TemplateText , dwFileSize, &dwBytesRead, NULL) == FALSE)
        return false;
    
    TemplateHead = TemplateText;

    LPTSTR szStart = wcsstr(TemplateText , _T("%images%"));
    if(szStart)
    {
        *szStart= 0;
        TemplateFoot = szStart+8;
    }
    CloseHandle(hFile);
    return true;
}

LRESULT CResultsPanel::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    TRC(IDC_IMAGEUPLOADERLABEL, "Images per string:");
    TRC(IDC_CODETYPELABEL, "Code type:");
    if(rectNeeded.left != -1)
    {
        SetWindowPos(  nullptr, rectNeeded.left, rectNeeded.top, rectNeeded.right,  rectNeeded.bottom, SWP_NOZORDER);
    }

    if (GetStyle() & WS_CHILD) {
        EnableThemeDialogTexture(m_hWnd, ETDT_ENABLETAB);
    }
    CBitmap hBitmap;

    //HIMAGELIST m_hToolBarImageList;
    DWORD rtlStyle = ServiceLocator::instance()->translator()->isRTL() ? ILC_MIRROR | ILC_PERITEMMIRROR : 0;
    const int iconWidth = GetSystemMetrics(SM_CXSMICON);
    const int iconHeight = GetSystemMetrics(SM_CYSMICON);

    toolbarImageList_.Create(iconWidth, iconHeight, (GuiTools::Is32BPP() ? ILC_COLOR32 : ILC_COLOR32 | ILC_MASK) | rtlStyle, 0, 6);
    
    auto loadToolbarIcon = [&](int resourceId) -> int {
        CIcon icon;
        icon.LoadIconWithScaleDown(MAKEINTRESOURCE(resourceId), iconWidth, iconHeight);
        return toolbarImageList_.AddIcon(icon);
    };

    if (ServiceLocator::instance()->translator()->isRTL()) {
        // Removing WS_EX_RTLREADING style from some controls to look properly when RTL interface language is chosen
        HWND codeEditHwnd = GetDlgItem(IDC_CODEEDIT);
        LONG styleEx = ::GetWindowLong(codeEditHwnd, GWL_EXSTYLE);
        ::SetWindowLong(codeEditHwnd, GWL_EXSTYLE, styleEx & ~WS_EX_RTLREADING);
    }

    CClientDC hdc(m_hWnd);
    float dpiScaleX_ = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    float dpiScaleY_ = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;

    RECT rc = {0,0,100,24};
    GetClientRect(&rc);
    rc.top = static_cast<LONG>(rc.bottom - dpiScaleY_ * 28);
    rc.bottom -= static_cast<LONG>(dpiScaleY_ * 4);
    rc.left = static_cast<LONG>(dpiScaleX_ * 8);
    rc.right -= static_cast<LONG>(dpiScaleX_ * 8);
    Toolbar.Create(m_hWnd,rc,_T(""), WS_CHILD|WS_CHILD | TBSTYLE_LIST |TBSTYLE_CUSTOMERASE|TBSTYLE_FLAT| CCS_NORESIZE/*|*/|CCS_BOTTOM | /*CCS_ADJUSTABLE|*/CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
    //TabBackgroundFix(Toolbar.m_hWnd);
    
    Toolbar.SetButtonStructSize();
    Toolbar.SetButtonSize(iconWidth+2, iconHeight +2);
    Toolbar.SetImageList(toolbarImageList_);
    Toolbar.AddButton(IDC_COPYALL, TBSTYLE_BUTTON|BTNS_AUTOSIZE ,TBSTATE_ENABLED, loadToolbarIcon(IDI_CLIPBOARD), TR("Copy to clipboard"), 0);
    
    //bool IsLastVideo = false; 

    Toolbar.AddButton(IDC_MEDIAFILEINFO, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, loadToolbarIcon(IDI_ICONINFO), TR("Info about last video"), 0);
    //Toolbar.AddButton(IDC_VIEWLOG, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 3, TR("Error log"), 0);
    Toolbar.AddButton(IDC_OPTIONSMENU, TBSTYLE_DROPDOWN |BTNS_AUTOSIZE, TBSTATE_ENABLED, loadToolbarIcon(IDI_ICONSETTINGSGEAR), TR("Options"), 0);
    Toolbar.AddButton(IDC_PREVIEWBUTTON, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, loadToolbarIcon(IDI_ICONPREVIEW), TR("Preview"), 0);
    
	//bool isMediaInfoAvailable = MediaInfoHelper::IsMediaInfoAvailable();

	//if (!IsLastVideo || !isMediaInfoAvailable) {
		Toolbar.HideButton(IDC_MEDIAFILEINFO);
	//}
        
    Toolbar.AutoSize();
    Toolbar.SetWindowLong(GWL_ID, IDC_RESULTSTOOLBAR);
    Toolbar.ShowWindow(SW_SHOW);

    codeTypeComboBox = GetDlgItem(IDC_CODETYPE);

    SetDlgItemInt(IDC_THUMBSPERLINE, settings->ThumbsPerLine);
    SendDlgItemMessage(IDC_THUMBPERLINESPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)0) );

    codeTypeComboBox.AddString(TR("Table of clickable thumbnails"));
    codeTypeComboBox.AddString(TR("Clickable thumbnails"));
    codeTypeComboBox.AddString(TR("Images"));
    codeTypeComboBox.AddString(TR("Links to Images/Files")); 
        
    for(size_t i=0;i<Templates.GetCount(); i++) {
        codeTypeComboBox.AddString(Templates[i].Name);  
    }
    
    codeTypeComboBox.SetCurSel(0);
    LoadTemplate();

    SetTimer(kOutputTimer, 1000);
    return 1;  // Let the system set the focus
}

LRESULT CResultsPanel::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (wParam == kOutputTimer) {
        if (outputChanged_) {
            code_ = GenerateOutput();
            SetDlgItemText(IDC_CODEEDIT, code_);
            outputChanged_ = false;
        }
    }
    
    return 0;
}

void CResultsPanel::SetPage(TabPage Index)
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    ::EnableWindow(GetDlgItem(IDC_CODETYPELABEL), Index != kPlainText);
    ::EnableWindow(GetDlgItem(IDC_CODETYPE), Index != kPlainText);
    ::EnableWindow(GetDlgItem(IDC_IMAGEUPLOADERLABEL), Index != kPlainText);
    
    ::EnableWindow(GetDlgItem(IDC_IMAGESPERLINELABEL), Index != kPlainText);
    ::EnableWindow(GetDlgItem(IDC_THUMBSPERLINE), Index != kPlainText);
    ::EnableWindow(GetDlgItem(IDC_THUMBPERLINESPIN), Index != kPlainText);
    
    bool enablePreview = Index != kMarkdown;
    Toolbar.EnableButton(IDC_PREVIEWBUTTON, enablePreview);
    m_Page = Index;


    UpdateOutput(true);
    BOOL temp;

    if (!openedFromHistory_ && !UrlList.empty() && Settings.AutoCopyToClipboard)
        OnBnClickedCopyall(0,0,0,temp);
}

void CResultsPanel::BBCode_Link(CString &Buffer, CUrlListItem &item)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    Buffer += _T("[url=");
    if(*item.getImageUrl(shortenUrl_) && (settings->UseDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty()))
        Buffer += item.getImageUrl(shortenUrl_);
    else 
        Buffer += item.getDownloadUrl(shortenUrl_);
    Buffer += _T("]");
    if (groupByFileName_) {
        Buffer += item.ServerName;
    } else {
        Buffer += WinUtils::myExtractFileName(item.FileName);
    }

    Buffer += _T("[/url]");

}

void CResultsPanel::HTML_Link(CString &Buffer, CUrlListItem &item)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    Buffer += _T("<a href=\"");
    if(*item.getImageUrl(shortenUrl_) && (settings->UseDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty()))
        Buffer += item.getImageUrl(shortenUrl_);
    else 
        Buffer += item.getDownloadUrl(shortenUrl_);
    Buffer += _T("\">");
    if (groupByFileName_) {
        Buffer += item.ServerName;
    } else {
        Buffer += WinUtils::myExtractFileName(item.FileName);
    }

    Buffer += _T("</a>");
}

void CResultsPanel::Markdown_Link(CString &Buffer, CUrlListItem &item)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    Buffer += _T("[");
    if (groupByFileName_) {
        Buffer += item.ServerName;
    } else {
        Buffer += WinUtils::myExtractFileName(item.FileName);
    }

    Buffer += _T("](");
    if (*item.getImageUrl(shortenUrl_) && (settings->UseDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty()))
        Buffer += item.getImageUrl(shortenUrl_);
    else
        Buffer += item.getDownloadUrl(shortenUrl_);
    Buffer += _T(")");
}

bool CResultsPanel::copyResultsToClipboard() {
    BOOL temp;
    OnBnClickedCopyall(0, 0, 0, temp);
    return true;
}

CString CResultsPanel::GenerateOutput()
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    CString Buffer;
    if (!Toolbar.m_hWnd) return _T("");
    int Index = GetCodeType();
    
    CodeType codeType = static_cast<CodeType>(Index);
    std::lock_guard<std::mutex> lock(UrlListCS);

    int n=UrlList.size();
    int p=GetDlgItemInt(IDC_THUMBSPERLINE);
    if(p>=0 && p<5555)
        settings->ThumbsPerLine = p;

    bool UseTemplate = settings->UseTxtTemplate;
    bool preferDirectLinks = settings->UseDirectLinks;
    groupByFileName_ = settings->GroupByFilename;
    settings->UseTxtTemplate = UseTemplate;
    if (UseTemplate && TemplateHead && m_Page != kPlainText) {
        Buffer += TemplateHead;
    }

    if (m_Page < kPlainText && Index > 3) //template from templates.xml
    {
        int TemplateIndex = Index - 4;
        CString Items;

        for(int i=0; i<n; i++)
        {
            if (UrlList[i].isNull())
            {
                continue;
            }
            CString fname= WinUtils::myExtractFileName(UrlList[i].FileName);
            CString imageUrl = UrlList[i].getImageUrl(shortenUrl_);
            m_Vars[_T("DownloadUrl")]=UrlList[i].getDownloadUrl(shortenUrl_);
            m_Vars[_T("ImageUrl")] = imageUrl.IsEmpty() ? UrlList[i].getDownloadUrl(shortenUrl_) : imageUrl;
            m_Vars[_T("ThumbUrl")]=UrlList[i].getThumbUrl(shortenUrl_);
            m_Vars[_T("FileName")]=fname;
            m_Vars[_T("FullFileName")]=UrlList[i].FileName;
            m_Vars[_T("Index")]= WinUtils::IntToStr(i);
            //CString buffer = WinUtils::GetOnlyFileName(UrlList[i].FileName);
            m_Vars[_T("FileNameWithoutExt")]=UrlList[i].FileName;
            if(p!=0  && !((i)%p))

                Items+=ReplaceVars(Templates[TemplateIndex].LineStart);
            Items+=ReplaceVars(Templates[TemplateIndex].Items);
            if((p!=0 && ((i+1)%p)) || p==0) 
                Items+=ReplaceVars(Templates[TemplateIndex].ItemSep);

            if(p!=0 && !((i+1)%p))
            {
                Items+=ReplaceVars(Templates[TemplateIndex].LineEnd);
                if(p!=0)
                    Items+=ReplaceVars(Templates[TemplateIndex].LineSep);
            }

        }
        m_Vars.clear();
        m_Vars["Items"]=Items;
        Buffer+=ReplaceVars(Templates[TemplateIndex].TemplateText);
        m_Vars.clear();
        return Buffer;
    }

    if(p<1) p=4;

    if (m_Page == kBbCode) {
        GenerateBBCode(Buffer, codeType, p, preferDirectLinks);
    } else if (m_Page == kHtml) {
        GenerateHTMLCode(Buffer, codeType, p, preferDirectLinks);
    } else if (m_Page == kMarkdown) {
        GenerateMarkdownCode(Buffer, codeType, p, preferDirectLinks);
    } else if (m_Page == kPlainText) {
        // Plaintext, just links
        std::vector<CString> groups;

        for (int i = 0; i < n; i++) {
            CUrlListItem& item = UrlList[i];
            if (item.isNull()) {
                continue;
            }
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
            }
            CString& cur = groups[index];
            if (*item.getImageUrl(shortenUrl_) && (preferDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty()))
                cur += item.getImageUrl(shortenUrl_);
            else
                cur += item.getDownloadUrl(shortenUrl_);
            if (i != n - 1)
                cur += _T("\r\n");
        }

        for (size_t i = 0; i < groups.size(); i++) {
            if (groupByFileName_ && !groups[i].IsEmpty() && i) {
                Buffer += _T("\r\n");   
            }
            Buffer += groups[i];
        }
    }

    if(UseTemplate && TemplateFoot && m_Page!=kPlainText)
        Buffer+=TemplateFoot;

    return Buffer;
}

void CResultsPanel::GenerateBBCode(CString& Buffer, CodeType codeType, int p /*thumbsPerLine*/, bool preferDirectLinks) {
    int n = UrlList.size();
    std::vector<CString> groups;
    std::vector<CString> fileNames;
    // Lang:BBCode, Type: "Table of clickable thumbnails" or "Clickable thumbnails"
    if (codeType == ctTableOfThumbnails || codeType == ctClickableThumbnails) {
        for (int i = 0; i < n; i++) {
            CUrlListItem& item = UrlList[i];
            if (item.isNull()) {
                continue;
            }
            CString fileNameWithoutPath = WinUtils::myExtractFileName(item.FileName);
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
                fileNames.resize(index + 1);
            }
            CString& cur = groups[index];
            fileNames[index] = fileNameWithoutPath;
            cur += _T("[url=");
            if (*item.getImageUrl(shortenUrl_) && (preferDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty()))
                cur += item.getImageUrl(shortenUrl_);
            else
                cur += item.getDownloadUrl(shortenUrl_);
            cur += _T("]");

            if (!item.getThumbUrl(shortenUrl_).IsEmpty()) {
                cur += _T("[img]");
                cur += item.getThumbUrl(shortenUrl_);
                cur += _T("[/img]");
            } else {
                cur += fileNameWithoutPath;
            }

            cur += _T("[/url]");

            if (codeType == ctTableOfThumbnails && ((i + 1) % p))
                cur += _T("  ");
            if (!((i + 1) % p) && codeType == 0 || codeType == 1)
                cur += _T("\r\n\r\n");
        }
    }

    // Lang: BBCode, Type: Full-sized images
    if (codeType == ctImages) {
        for (int i = 0; i < n; i++) {
            CUrlListItem& item = UrlList[i];
            if (item.isNull()) {
                continue;
            }
            CString fileNameWithoutPath = WinUtils::myExtractFileName(item.FileName);
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
                fileNames.resize(index + 1);
            }
            CString& cur = groups[index];
            fileNames[index] = fileNameWithoutPath;

            if (*item.getImageUrl(shortenUrl_) && (preferDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty())) {
                cur += _T("[img]");
                cur += item.getImageUrl(shortenUrl_);
                cur += _T("[/img]");
            } else BBCode_Link(cur, item);
            cur += _T("\r\n\r\n");
        }
    }

    // Lang: BBCode, Type: Links to Images/Files
    if (codeType == ctLinks) {
        for (int i = 0; i < n; i++) {
            if (UrlList[i].isNull()) {
                continue;
            }
            CUrlListItem& item = UrlList[i];
            CString fileNameWithoutPath = WinUtils::myExtractFileName(item.FileName);
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
                fileNames.resize(index + 1);
            }
            CString& cur = groups[index];
            fileNames[index] = fileNameWithoutPath;

            BBCode_Link(cur, UrlList[i]);

            cur += _T("\r\n");
        }
    }

    for (size_t i = 0; i < groups.size(); i++) {
        if (groupByFileName_ && !groups[i].IsEmpty()) {
            if (i) {
                Buffer += _T("\r\n");
            }
            Buffer += fileNames[i] + _T("\r\n\r\n");
        }
        Buffer += groups[i];
    }
}

void CResultsPanel::GenerateHTMLCode(CString& Buffer, CodeType codeType, int p /*thumbsPerLine*/, bool preferDirectLinks) {
    int n = UrlList.size();
    std::vector<CString> groups;
    std::vector<CString> fileNames;

    // Lang: HTML, Type: "Table of clickable thumbnails" or "Clickable thumbnails"
    if (codeType == ctTableOfThumbnails || codeType == ctClickableThumbnails) {
        for (int i = 0; i<n; i++) {
            CUrlListItem& item = UrlList[i];
            if (item.isNull()) {
                continue;
            }
            CString fileNameWithoutPath = WinUtils::myExtractFileName(item.FileName);
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
                fileNames.resize(index + 1);
            }
            CString& cur = groups[index];
            fileNames[index] = fileNameWithoutPath;

            cur += _T("<a href=\"");
            if (*item.getImageUrl(shortenUrl_) && (preferDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty()))
                cur += item.getImageUrl(shortenUrl_);
            else
                cur += item.getDownloadUrl(shortenUrl_);
            cur += _T("\">");
            if (!item.getThumbUrl(shortenUrl_).IsEmpty()) {
                cur += _T("<img src=\"");
                cur += item.getThumbUrl(shortenUrl_);
                cur += _T("\" alt=\"\">");
            } else
                cur += fileNameWithoutPath;
            cur += _T("</a>");
            if (((i + 1) % p) && codeType == ctTableOfThumbnails)
                cur += _T("&nbsp;&nbsp;");
            if (!((i + 1) % p) && codeType == ctTableOfThumbnails || codeType == ctClickableThumbnails)
                cur += _T("<br/>&nbsp;<br/>\r\n");
        }  
    }

    // Lang: HTML, Type: Full-sized images
    if (codeType == ctImages) {
        for (int i = 0; i<n; i++) {
            CUrlListItem& item = UrlList[i];
            if (item.isNull()) {
                continue;
            }
            CString fileNameWithoutPath = WinUtils::myExtractFileName(item.FileName);
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
                fileNames.resize(index + 1);
            }
            CString& cur = groups[index];
            fileNames[index] = fileNameWithoutPath;
            if (!item.getImageUrl(shortenUrl_).IsEmpty() && (preferDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty())) {
                cur += _T("<img src=\"");
                cur += item.getImageUrl(shortenUrl_);
                cur += _T("\" alt=\"\">");
            } else {
                HTML_Link(cur, UrlList[i]);
            }
            cur += _T("<br/>&nbsp;<br/>");
        }
    }

    // Lang: HTML, Type: Links to Images/Files
    if (codeType == ctLinks) {
        for (int i = 0; i<n; i++) {
            CUrlListItem& item = UrlList[i];
            if (item.isNull()) {
                continue;
            }
            CString fileNameWithoutPath = WinUtils::myExtractFileName(item.FileName);
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
                fileNames.resize(index + 1);
            }
            CString& cur = groups[index];
            //if (i) {
                
            //}
            
            fileNames[index] = fileNameWithoutPath;
            HTML_Link(cur, UrlList[i]);
            if (i != n - 1) {
                cur += _T("<br/>");
            }
        }
    }

    for (size_t i = 0; i < groups.size(); i++) {
        if (groupByFileName_ && !groups[i].IsEmpty()) {
            if (i) {
                Buffer += _T("<br/>");
            }
            Buffer += fileNames[i] + _T("<br/>");
        }
        Buffer += groups[i];
    }
}

void CResultsPanel::GenerateMarkdownCode(CString& Buffer, CodeType codeType, int p /*thumbsPerLine*/, bool preferDirectLinks) {
    int n = UrlList.size();
    std::vector<CString> groups;
    std::vector<CString> fileNames;

    // Lang:Markdown, Type: "Table of clickable thumbnails" or "Clickable thumbnails"
    if (codeType == ctTableOfThumbnails || codeType == ctClickableThumbnails) {
        for (int i = 0; i < n; i++) {
            CUrlListItem& item = UrlList[i];
            if (item.isNull()) {
                continue;
            }
            CString fileNameWithoutPath = WinUtils::myExtractFileName(item.FileName);
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
                fileNames.resize(index + 1);
            }
            CString& cur = groups[index];
            fileNames[index] = fileNameWithoutPath;

            CString linkUrl, linkText;
            if (*item.getImageUrl(shortenUrl_) && (preferDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty()))
                linkUrl = item.getImageUrl(shortenUrl_);
            else
                linkUrl = item.getDownloadUrl(shortenUrl_);

            if (!item.getThumbUrl(shortenUrl_).IsEmpty()) {
                linkText = _T("![](");
                linkText += item.getThumbUrl(shortenUrl_);
                linkText += _T(")");
            } else {
                linkText = fileNameWithoutPath;
            }
            CString line;
            line.Format(_T("[%s](%s)"), static_cast<LPCTSTR>(linkText), static_cast<LPCTSTR>(linkUrl));
            cur += line;

            if (codeType == ctTableOfThumbnails && ((i + 1) % p))
                cur += _T("  ");
            if (!((i + 1) % p) && codeType == 0 || codeType == 1)
                cur += _T("\r\n\r\n");
        }
    }

    // Lang: Markdown, Type: Full-sized images
    if (codeType == ctImages) {
        for (int i = 0; i < n; i++) {
            CUrlListItem& item = UrlList[i];
            if (item.isNull()) {
                continue;
            }
            CString fileNameWithoutPath = WinUtils::myExtractFileName(item.FileName);
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
                fileNames.resize(index + 1);
            }
            CString& cur = groups[index];
            fileNames[index] = fileNameWithoutPath;

            if (*item.getImageUrl(shortenUrl_) && (preferDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty())) {
                cur += _T("![");
                cur += WinUtils::myExtractFileName(item.FileName);
                cur += _T("](");
                cur += item.getImageUrl(shortenUrl_);
                cur += _T(")");
            } else Markdown_Link(cur, item);
            cur += _T("\r\n\r\n");
        }
    }

    // Lang: Markdown, Type: Links to Images/Files
    if (codeType == ctLinks) {
        for (int i = 0; i < n; i++) {
            CUrlListItem& item = UrlList[i];
            if (item.isNull()) {
                continue;
            }
            CString fileNameWithoutPath = WinUtils::myExtractFileName(item.FileName);
            size_t index = item.FileIndex;
            if (groups.size() <= index) {
                groups.resize(index + 1);
                fileNames.resize(index + 1);
            }
            CString& cur = groups[index];
            fileNames[index] = fileNameWithoutPath;
            Markdown_Link(cur, UrlList[i]);

            cur += _T("\r\n");
        }
    }

    for (size_t i = 0; i < groups.size(); i++) {
        if (groupByFileName_ && !groups[i].IsEmpty()) {
            if (i) {
                Buffer += _T("\r\n");
            }
            Buffer += fileNames[i] + _T("\r\n\r\n");
        }
        Buffer += groups[i];
    }
}

void CResultsPanel::UpdateOutput(bool immediately)
{
    if (immediately) {
        code_ = GenerateOutput();
        SetDlgItemText(IDC_CODEEDIT, code_);
    } else {
        outputChanged_ = true;
    }
}


LRESULT CResultsPanel::OnCbnSelchangeCodetype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    onCodeTypeChanged();
    return 0;
}

void CResultsPanel::onCodeTypeChanged() {
    UpdateOutput(true);
    BOOL temp;
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    if (settings->AutoCopyToClipboard) {
        OnBnClickedCopyall(0, 0, 0, temp);
    }
}

LRESULT CResultsPanel::OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CString buffer = GenerateOutput();
    WinUtils::CopyTextToClipboard(buffer);
    if (m_Page == kHtml && !buffer.IsEmpty()) {
        WinUtils::CopyHtmlToClipboard(buffer);
    }
    return 0;
}

// "Thumb per line" edit text changed event
LRESULT CResultsPanel::OnEditChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    onCodeTypeChanged();
    return 0;
 }

LRESULT CResultsPanel::OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
#ifdef IU_ENABLE_MEDIAINFO
    if (WizardDlg->getLastVideoFile().IsEmpty()) {
        return 0;
    }
    CMediaInfoDlg dlg;
    dlg.ShowInfo(m_hWnd, WizardDlg->getLastVideoFile());
#endif
    return 0;
}

    
int CResultsPanel::GetCodeType() const {
    return codeTypeComboBox.GetCurSel();
}

void CResultsPanel::SetCodeType(int Index) {
    codeTypeComboBox.SetCurSel(Index);
    onCodeTypeChanged();
}

void CResultsPanel::Clear()
{
    SetDlgItemText(IDC_CODEEDIT, _T(" "));
}

void  CResultsPanel::EnableMediaInfo(bool Enable)
{
    Toolbar.HideButton(IDC_MEDIAFILEINFO,!Enable);
}

bool CResultsPanel::LoadTemplates(CString &Error)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    CString XmlFileName = IuCommonFunctions::GetDataFolder() + _T("templates.xml");
    LoadTemplateFromFile(XmlFileName, Error);
    CString userTemplateError;
    CString userTemplateFile = Utf8ToWCstring(settings->SettingsFolder) +  _T("user_templates.xml");
    LoadTemplateFromFile(userTemplateFile, userTemplateError);

    return true;
}

bool CResultsPanel::LoadTemplateFromFile(const CString& fileName, CString &Error)
{
    SimpleXml XML;
    if(!WinUtils::FileExists(fileName))
    {
        Error = TR("File not found.");
        return false;
    }

    if(!XML.LoadFromFile(WCstringToUtf8(fileName)))
    {
        Error = _T("xml loading error");
        return false;
    }

    SimpleXmlNode templatesNode = XML.getRoot("Templates");
    if(templatesNode.IsNull())
    {
        Error = _T("Unable to find Templates node");
        return false;
    }

    std::vector<SimpleXmlNode> templates;
    templatesNode.GetChilds("Template",templates);

    for(size_t i=0; i<templates.size(); i++)
    {
        IU_Result_Template Template;
        Template.Name = Utf8ToWCstring(templates[i].Attribute("Name"));

        Template.TemplateText = Utf8ToWCstring(templates[i]["Text"].Text());

        SimpleXmlNode itemsNode = templates[i]["Items"];
        if(!itemsNode.IsNull())
        {
            Template.LineStart = Utf8ToWCstring(itemsNode.Attribute("LineStart"));
            Template.LineEnd = Utf8ToWCstring(itemsNode.Attribute("LineEnd"));
            Template.LineSep = Utf8ToWCstring(itemsNode.Attribute("LineSep"));
            Template.ItemSep = Utf8ToWCstring(itemsNode.Attribute("ItemSep"));

            Template.Items = Utf8ToWCstring(itemsNode.Text());
        }

        Templates.Add(Template);
    }
    return true;
}

CString CResultsPanel::ReplaceVars(const CString& Text)
{
    CString Result =  Text;

    pcrepp::Pcre reg("\\$\\(([A-z0-9_]*?)\\)", "imc");
    std::string str = WCstringToUtf8(Text);
    size_t pos = 0;
    while (pos <= str.length()) 
    {
        if( reg.search(str, pos)) 
        {
            pos = reg.get_match_end()+1;
            CString vv = Utf8ToWstring(reg[1]).c_str();
            /*if(!vv.IsEmpty() && vv[0] == _T('_'))
                Result.Replace(CString(_T("$(")) + vv + _T(")"),m_Consts[vv]);
            else*/
                Result.Replace(CString(_T("$(")) + vv + _T(")"),m_Vars[vv]);
        }
        else
            break;
    }
    Result.Replace(L"\\n",L"\r\n");

    return Result;
}

LRESULT CResultsPanel::OnOptionsDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    auto* pnmtb = reinterpret_cast<NMTOOLBAR *>(pnmh);
    CMenu sub;    
    MENUITEMINFO mi;
    mi.cbSize = sizeof(mi);
    mi.fMask = MIIM_TYPE|MIIM_ID;
    mi.fType = MFT_STRING;
    sub.CreatePopupMenu();
    RECT rc;
    
    ::GetWindowRect(GetDlgItem(IDC_RESULTSTOOLBAR),&rc); 
    int count = 0;
        mi.fType = MFT_STRING;
        mi.wID = IDC_SHORTENURLITEM;
        CString menuItemTitle;
        if (onShortenUrlChanged_) {
            menuItemTitle.Format(TR("Shorten URL using %s"), IuCoreUtils::Utf8ToWstring(settings->urlShorteningServer.serverName()).c_str());
        } else {
            menuItemTitle = TR("Shorten URL");
        }
        mi.dwTypeData  = const_cast<LPWSTR>(menuItemTitle.GetString());
        sub.InsertMenuItem(count++, true, &mi);
    

    mi.fType = MFT_STRING;
    mi.wID = IDC_USEDIRECTLINKS;
    CString useDirectLinks = TR("Use direct links");
    mi.dwTypeData = const_cast<LPWSTR>(useDirectLinks.GetString());
    sub.InsertMenuItem(count++, true, &mi);

    mi.fType = MFT_STRING;
    mi.wID = IDC_GROUPBYFILENAME;
    CString groupByFilename = TR("Group by filename");
    mi.dwTypeData = const_cast<LPWSTR>(groupByFilename.GetString());
    sub.InsertMenuItem(count++, true, &mi);

    mi.wID = IDC_USETEMPLATE;
    CString useTemplate = TR("Use template");
    mi.dwTypeData = const_cast<LPWSTR>(useTemplate.GetString());
    sub.InsertMenuItem(count++, true, &mi);

    int insertedServersCount = 0;
    int curPosition = count;

    for(size_t i=0; i<m_Servers.size(); i++)
    {
        CUploadEngineData *ue = m_Servers[i].uploadEngineData();
        if(!ue) continue;
        CString folderTitle = Utf8ToWCstring(m_Servers[i].folderTitle());
        CString folderUrl = Utf8ToWCstring(m_Servers[i].folderUrl());

        if(folderTitle.IsEmpty() || folderUrl.IsEmpty()) continue;
        CString title = TR("Copy URL  ") + Utf8ToWCstring(ue->Name)+ _T("->")+folderTitle;
        mi.wID = IDC_COPYFOLDERURL + i;
        mi.dwTypeData = const_cast<LPWSTR>(title.GetString());
        sub.InsertMenuItem(count++, true, &mi);
        insertedServersCount++;
    }

    if(insertedServersCount)
    {
        mi.wID = IDC_FILESERVER_LAST_ID + 1;
        mi.fType = MFT_SEPARATOR;
        sub.InsertMenuItem(curPosition, true, &mi);
        count++;
    }

    sub.CheckMenuItem(IDC_USEDIRECTLINKS, MF_BYCOMMAND    | (settings->UseDirectLinks? MF_CHECKED    : MF_UNCHECKED)    );
    sub.CheckMenuItem(IDC_USETEMPLATE, MF_BYCOMMAND    | (settings->UseTxtTemplate? MF_CHECKED    : MF_UNCHECKED)    );
    sub.CheckMenuItem(IDC_SHORTENURLITEM, MF_BYCOMMAND    | (shortenUrl_? MF_CHECKED    : MF_UNCHECKED)    );    
    sub.CheckMenuItem(IDC_GROUPBYFILENAME, MF_BYCOMMAND    | (groupByFileName_? MF_CHECKED    : MF_UNCHECKED)    );
    
    ::SendMessage(Toolbar.m_hWnd,TB_GETRECT, pnmtb->iItem, reinterpret_cast<LPARAM>(&rc));
    Toolbar.ClientToScreen(&rc);
    TPMPARAMS excludeArea;
    ZeroMemory(&excludeArea, sizeof(excludeArea));
    excludeArea.cbSize = sizeof(excludeArea);
    excludeArea.rcExclude = rc;
    sub.TrackPopupMenuEx( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
    rc.left, rc.bottom, m_hWnd, /*&rc, */&excludeArea);
    bHandled = true;
    return TBDDRET_DEFAULT;
}

LRESULT CResultsPanel::OnUseTemplateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    settings->UseTxtTemplate = !settings->UseTxtTemplate;
    UpdateOutput(true);
    return 0;
}

LRESULT CResultsPanel::OnUseDirectLinksClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    settings->UseDirectLinks = !settings->UseDirectLinks;
    UpdateOutput(true);
    return 0;
}

LRESULT CResultsPanel::OnCopyFolderUrlClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int index = wID - IDC_COPYFOLDERURL;

    CUploadEngineData *ue = m_Servers[index].uploadEngineData();
    if(!ue) return 0;
    CString folderUrl = Utf8ToWCstring( m_Servers[index].folderUrl());
    WinUtils::CopyTextToClipboard(folderUrl);
    return 0;
}

void CResultsPanel::AddServer(const ServerProfile& server)
{
    for (const auto& cur : m_Servers) {
        if (cur.serverName() == server.serverName() && cur.profileName() == server.profileName()
                && cur.folderId() == server.folderId()) {
            return;
        }
    }
    m_Servers.push_back(server);
    //return 0;
}
LRESULT CResultsPanel::OnResulttoolbarNMCustomDraw(LPNMHDR pnmh)
{
    auto* lpNMCustomDraw = reinterpret_cast<LPNMTBCUSTOMDRAW>(pnmh);
    HDC dc = lpNMCustomDraw->nmcd.hdc;
    RECT toolbarRect = lpNMCustomDraw->nmcd.rc;
    //HTHEME hTheme = OpenThemeData(m_hWnd, _T("TAB"));
    RECT rc;
    GetClientRect(&rc);


    // Check if the application is themed

    if (IsAppThemed()) {
        //rc.top-=10;

        //m_wndTab.GetWindowRect(&rc);

        // Get the tab control DC

        //  HDC hDC = m_wndTab.GetDC();

        // Create a compatible DC

        HDC hDCMem = ::CreateCompatibleDC(dc);
        HBITMAP hBmp = ::CreateCompatibleBitmap(dc, rc.right - rc.left, rc.bottom - rc.top);
        HBITMAP hBmpOld = static_cast<HBITMAP>(::SelectObject(hDCMem, hBmp));

        // Tell the tab control to paint in our DC

        /* m_wndTab.*/
        SendMessage(WM_PRINTCLIENT, reinterpret_cast<WPARAM>(hDCMem), PRF_ERASEBKGND | PRF_CLIENT | PRF_NONCLIENT);

        ::MapWindowPoints(Toolbar.m_hWnd, m_hWnd, reinterpret_cast<LPPOINT>(&toolbarRect), 2);
        BitBlt(dc, 0, 0, toolbarRect.right - toolbarRect.left, toolbarRect.bottom - toolbarRect.top, hDCMem, toolbarRect.left, toolbarRect.top,SRCCOPY);

        SelectObject(hDCMem, hBmpOld);
        DeleteObject(hBmp);
        DeleteDC(hDCMem);
    }

    return CDRF_DODEFAULT; // Default handler
}


void CResultsPanel::setEngineList(CMyEngineList* EngineList)
{
    m_EngineList = EngineList;
}

void CResultsPanel::InitUpload()
{
    m_Servers.clear();
    shortenUrl_ = false;
}

LRESULT CResultsPanel::OnShortenUrlClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    shortenUrl_ = !shortenUrl_;
    if (onShortenUrlChanged_) {
        onShortenUrlChanged_(shortenUrl_);
    }
    return 0;
}


LRESULT CResultsPanel::OnPreviewButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    CString url ;
    if ( m_Page == 2 && this->UrlList.size() ) {
        //use
        url = this->UrlList[0].getImageUrl();
        if ((!Settings.UseDirectLinks || url.IsEmpty()) &&  !this->UrlList[0].getDownloadUrl().IsEmpty()) {
            url = this->UrlList[0].getDownloadUrl();
        }
    } else {
        CString outputTempFileName = AppParams::instance()->tempDirectoryW()  + "preview.html";
        CString code = GenerateOutput();
        /*if ( m_Page == 0) {
            code = Utf8ToWCstring(IuTextUtils::BbCodeToHtml(WCstringToUtf8(code)));
        } */
        std::string res = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" />"
            "<style>img {border:none;}</style>"
            "</head><body>";
        res += "<span id=\"resultcode\">" + W2U(code) + "</span>";
        if (m_Page == 0) {
            std::string script;
            std::string scriptFileName = W2U(IuCommonFunctions::GetDataFolder()) + "Utils/xbbcode.js";
            if (!IuCoreUtils::ReadUtf8TextFile(scriptFileName, script))
            {
                LOG(ERROR) << "Cannot read file " << scriptFileName;
            }
            res += "<script type=\"text/javascript\">" + script +
                "\r\n"
                "var el = document.getElementById('resultcode');\r\n"
                "var result = XBBCODE.process({   \r\n"
                "   text: el.innerHTML,   \r\n"
                "   removeMisalignedTags: false,  \r\n"
                "   addInLineBreaks: true   \r\n"
                "});  \r\n"
                "el.innerHTML = result.html; \r\n"
                "</script>";
        }
       res+= "</body></html>";

        if (!IuTextUtils::FileSaveContents(W2U(outputTempFileName), res) ) {
            LOG(ERROR) << "Could not save temporary file " << outputTempFileName;
        }
        url = "file:///" + outputTempFileName;   
    }

    if (url.IsEmpty() ) {
        return 0;
    }

    CRect r(0,0,600,400);
    if ( !webViewWindow_ ) {
        webViewWindow_ = std::make_unique<CWebViewWindow>();
        webViewWindow_->Create(0,r,TR("Preview Window"),WS_POPUP|WS_OVERLAPPEDWINDOW,WS_EX_TOPMOST    );
        webViewWindow_->CenterWindow(WizardDlg->m_hWnd);
        webViewWindow_->ShowWindow(SW_SHOW);
    }
    
    webViewWindow_->NavigateTo(url);
    webViewWindow_->ShowWindow(SW_SHOW);
//    webViewWindow_->ActivateWindow();
    return 0;
}

std::mutex& CResultsPanel::outputMutex() {
    return UrlListCS;
}

void CResultsPanel::setRectNeeded(const RECT& rc) {
    rectNeeded = rc;
}

void CResultsPanel::setShortenUrls(bool shorten) {
    shortenUrl_ = shorten;
}

void CResultsPanel::setOnShortenUrlChanged(ShortenUrlChangedCallback callback) {
    onShortenUrlChanged_ = std::move(callback);
}

void CResultsPanel::setGroupByFilename(bool enable) {
    groupByFileName_ = enable;
}

LRESULT CResultsPanel::OnGroupByFilenameClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    groupByFileName_ = !groupByFileName_;
    settings->GroupByFilename = groupByFileName_;
    UpdateOutput(true);
    return 0;
}
