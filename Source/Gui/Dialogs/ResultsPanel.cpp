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

#include "ResultsPanel.h"

#include "Core/3rdpart/pcreplusplus.h"
#include "UploadSettings.h"
#include "mediainfodlg.h"
#include "LogWindow.h"
#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Gui/Dialogs/WebViewWindow.h"
#include "Core/Utils/TextUtils.h"
#include "Func/IuCommonFunctions.h"
#include "Func/WinUtils.h"
#include "Core/ServiceLocator.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "Func/MediaInfoHelper.h"

// CResultsPanel
CResultsPanel::CResultsPanel(CWizardDlg *dlg, std::vector<CUrlListItem>  & urlList, bool openedFromHistory) :WizardDlg(dlg), UrlList(urlList)
{
    webViewWindow_ = NULL;
    m_nImgServer = m_nFileServer = -1;
    TemplateHead = TemplateFoot = NULL; 
    openedFromHistory_ = openedFromHistory;
    rectNeeded.left = -1;
    CString TemplateLoadError;
    shortenUrl_ = false;
    if(!LoadTemplates(TemplateLoadError))
    {
        ServiceLocator::instance()->logger()->write(logWarning, _T("Results Module"), TemplateLoadError);
    }
}

CResultsPanel::~CResultsPanel()
{
    if(TemplateHead) delete[] TemplateHead;
    if (webViewWindow_ && webViewWindow_->m_hWnd)
    {
        webViewWindow_->DestroyWindow();
        delete webViewWindow_;
    }  
}
    
bool CResultsPanel::LoadTemplate()
{
    if(TemplateHead && TemplateFoot) return true;

    DWORD dwBytesRead, dwFileSize;
    CString FileName = IuCommonFunctions::GetDataFolder() + _T("template.txt");

    if(TemplateHead) delete[] TemplateHead;
    TemplateHead = NULL;
    TemplateFoot = NULL;
    HANDLE hFile = CreateFile(FileName, GENERIC_READ, 0, 0,OPEN_EXISTING, 0, 0);
    if(!hFile) return false;

    dwFileSize = GetFileSize (hFile, NULL) ; 
    if(!dwFileSize) return false;
    DWORD dwMemoryNeeded = min(35536ul, dwFileSize);
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
    TRC(IDC_IMAGEUPLOADERLABEL, "Images per string:");
    TRC(IDC_CODETYPELABEL, "Code type:");
    if(rectNeeded.left != -1)
    {
    
    SetWindowPos(  0, rectNeeded.left, rectNeeded.top, rectNeeded.right,  rectNeeded.bottom, 0);
    }
    if(GetStyle()&WS_CHILD)
    TabBackgroundFix(m_hWnd);
    CBitmap hBitmap;

    HIMAGELIST m_hToolBarImageList;
    if (Is32BPP())
    {
        hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_BITMAP3));
        
        m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32,0,6);
        ImageList_Add(m_hToolBarImageList,hBitmap,NULL);
    }
    else
    {
        hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_BITMAP4));

        m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
        ImageList_AddMasked(m_hToolBarImageList,hBitmap,RGB(255,0,255));
    }
    CDC hdc = GetDC();
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
    Toolbar.SetButtonSize(30,18);
    Toolbar.SetImageList(m_hToolBarImageList);
    Toolbar.AddButton(IDC_COPYALL, TBSTYLE_BUTTON|BTNS_AUTOSIZE ,TBSTATE_ENABLED, 0, TR("Copy to clipboard"), 0);
    
    bool IsLastVideo = false; 

    Toolbar.AddButton(IDC_MEDIAFILEINFO, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("Info about last video"), 0);
    Toolbar.AddButton(IDC_VIEWLOG, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 3, TR("Error log"), 0);
    Toolbar.AddButton(IDC_OPTIONSMENU, TBSTYLE_DROPDOWN |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, TR("Options"), 0);
    Toolbar.AddButton(IDC_PREVIEWBUTTON, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 4, TR("Preview"), 0);
    
	bool isMediaInfoAvailable = MediaInfoHelper::IsMediaInfoAvailable();

	if (!IsLastVideo || !isMediaInfoAvailable) {
		Toolbar.HideButton(IDC_MEDIAFILEINFO);
	}
        
    Toolbar.AutoSize();
    Toolbar.SetWindowLong(GWL_ID, IDC_RESULTSTOOLBAR);
    Toolbar.ShowWindow(SW_SHOW);

    SetDlgItemInt(IDC_THUMBSPERLINE, Settings.ThumbsPerLine);
    SendDlgItemMessage(IDC_THUMBPERLINESPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)0) );
    SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(TR("Table of clickable thumbnails")));
    SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(TR("Clickable thumbnails")));
    SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(TR("Images")));
    SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(TR("Links to Images/Files")));    
        
    for(size_t i=0;i<Templates.GetCount(); i++)
    {
        SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)Templates[i].Name);    
    }
    
    SendDlgItemMessage(IDC_CODETYPE,CB_SETCURSEL, 0);
    LoadTemplate();
    return 1;  // Let the system set the focus
}

void CResultsPanel::SetPage(TabPage Index)
{
    ::EnableWindow(GetDlgItem(IDC_CODETYPELABEL), Index!=2);
    ::EnableWindow(GetDlgItem(IDC_CODETYPE), Index!=2);
    ::EnableWindow(GetDlgItem(IDC_IMAGEUPLOADERLABEL), Index!=2);
    
    ::EnableWindow(GetDlgItem(    IDC_IMAGESPERLINELABEL), Index!=2);
    ::EnableWindow(GetDlgItem(IDC_THUMBSPERLINE), Index!=2);
    ::EnableWindow(GetDlgItem(IDC_THUMBPERLINESPIN), Index!=2);
    
    m_Page = Index;

    
    UpdateOutput();
    BOOL temp;

    if (!openedFromHistory_ && !UrlList.empty() && Settings.AutoCopyToClipboard)
        OnBnClickedCopyall(0,0,0,temp);
}

void CResultsPanel::BBCode_Link(CString &Buffer, CUrlListItem &item)
{
    Buffer += _T("[url=");
    if(*item.getImageUrl(shortenUrl_) && (Settings.UseDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty()))
        Buffer += item.getImageUrl(shortenUrl_);
    else 
        Buffer += item.getDownloadUrl(shortenUrl_);
    Buffer += _T("]");
    Buffer += WinUtils::myExtractFileName(item.FileName);
    Buffer += _T("[/url]");

}

void CResultsPanel::HTML_Link(CString &Buffer, CUrlListItem &item)
{
    Buffer += _T("<a href=\"");
    if(*item.getImageUrl(shortenUrl_) && (Settings.UseDirectLinks || item.getDownloadUrl(shortenUrl_).IsEmpty()))
        Buffer += item.getImageUrl(shortenUrl_);
    else 
        Buffer += item.getDownloadUrl(shortenUrl_);
    Buffer += _T("\">");
    Buffer += WinUtils::myExtractFileName(item.FileName);
    Buffer += _T("</a>");
}

const CString CResultsPanel::GenerateOutput()
{
    CString Buffer;
    if(!Toolbar.m_hWnd) return _T("");
    int Index =    GetCodeType();

    int type=0;

    if(m_Page==0)
        type=Index;
    if (m_Page==1) 
        type= 4+Index;
    if(m_Page==2)
        type=8;

    UrlListCS.Lock();

    int n=UrlList.size();
    int p=GetDlgItemInt(IDC_THUMBSPERLINE);
    if(p>=0 && p<5555)
        Settings.ThumbsPerLine = p;

    bool UseTemplate = Settings.UseTxtTemplate;
    //Toolbar.IsButtonChecked(IDC_USETEMPLATE); //IsChecked(IDC_USETEMPLATE);
    Settings.UseTxtTemplate = UseTemplate;
    if(UseTemplate && TemplateHead && m_Page!=2)
        Buffer+=TemplateHead;


    if(m_Page<2 && Index>3) //template from templates.xml
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
            CString buffer;
            buffer = WinUtils::GetOnlyFileName(UrlList[i].FileName);
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
        UrlListCS.Unlock();
        return Buffer;
    }

    if(p<1) p=4;

    if(type==0||type==1)
    {
        for(int i=0;i<n;i++)
        {
            if (UrlList[i].isNull())
            {
                continue;
            }
            Buffer+=_T("[url=");
            if(*UrlList[i].getImageUrl(shortenUrl_)&& (Settings.UseDirectLinks || UrlList[i].getDownloadUrl(shortenUrl_).IsEmpty()))

                Buffer+=UrlList[i].getImageUrl(shortenUrl_);
            else 
                Buffer+=UrlList[i].getDownloadUrl(shortenUrl_);
            Buffer+=_T("]");

            if(lstrlen(UrlList[i].getThumbUrl(shortenUrl_))>0)
            {
                Buffer+=_T("[img]");
                Buffer+=UrlList[i].getThumbUrl(shortenUrl_);
                Buffer+=_T("[/img]");
            }
            else
            {
                Buffer+= WinUtils::myExtractFileName(UrlList[i].FileName);
            }

            Buffer+=_T("[/url]");


            if(type==0&&((i+1)%p))
                Buffer+=_T("  ");
            if(!((i+1)%p)&&type==0||type==1)
                Buffer+=_T("\r\n\r\n");
        }
    }

    if(type==2)
    {
        for(int i=0;i<n;i++)
        {
            if (UrlList[i].isNull())
            {
                continue;
            }
            if(*UrlList[i].getImageUrl(shortenUrl_)&& (Settings.UseDirectLinks || UrlList[i].getDownloadUrl(shortenUrl_).IsEmpty()))
            {
                Buffer+=_T("[img]");
                Buffer+=UrlList[i].getImageUrl(shortenUrl_);
                Buffer+=_T("[/img]");
            }
            else BBCode_Link(Buffer,UrlList[i]);
            Buffer+=_T("\r\n\r\n");
        }
    }

    if(type==3)
    {
        for(int i=0;i<n;i++)
        {
            if (UrlList[i].isNull())
            {
                continue;
            }
            BBCode_Link(Buffer, UrlList[i]);

            Buffer+=_T("\r\n");
        }
    }
    if(type==8)
    {
        for(int i=0;i<n;i++)
        {
            if (UrlList[i].isNull())
            {
                continue;
            }
            if(*UrlList[i].getImageUrl(shortenUrl_)&& (Settings.UseDirectLinks || UrlList[i].getDownloadUrl(shortenUrl_).IsEmpty())) 
                Buffer+=UrlList[i].getImageUrl(shortenUrl_);
            else 
                Buffer+=UrlList[i].getDownloadUrl(shortenUrl_);
            if(i != n-1)
            Buffer+=_T("\r\n");
        }
    }

    if(type==4  || type==5)
    {
        Buffer+=_T("<center>");
        for(int i=0;i<n;i++)
        {
            if (UrlList[i].isNull())
            {
                continue;
            }
            Buffer+=_T("<a href=\"");
            if(*UrlList[i].getImageUrl(shortenUrl_)&& (Settings.UseDirectLinks || UrlList[i].getDownloadUrl(shortenUrl_).IsEmpty())) 
                Buffer+=UrlList[i].getImageUrl(shortenUrl_);
            else 
                Buffer+=UrlList[i].getDownloadUrl(shortenUrl_);
            Buffer+=_T("\">");
            if(lstrlen(UrlList[i].getThumbUrl(shortenUrl_))>0)
            {
                Buffer+=_T("<img src=\"");
                Buffer+=UrlList[i].getThumbUrl(shortenUrl_);
                Buffer+=_T("\" border=0>");
            }
            else
                Buffer+= WinUtils::myExtractFileName(UrlList[i].FileName);
            Buffer+=_T("</a>");
            if(((i+1)%p)&&type==4)
                Buffer+=_T("&nbsp;&nbsp;");
            if(!((i+1)%p) &&type==4||type==5 )
                Buffer+=_T("<br/>&nbsp;<br/>\r\n");
        }
        Buffer+=_T("</center>");
    }

    if(type == 6)
    {
        for(int i=0; i<n; i++)
        {
            if (UrlList[i].isNull())
            {
                continue;
            }
            if(lstrlen(UrlList[i].getImageUrl(shortenUrl_))>0 && (Settings.UseDirectLinks || UrlList[i].getDownloadUrl(shortenUrl_).IsEmpty()))
            {
                Buffer += _T("<img src=\"");
                Buffer += UrlList[i].getImageUrl(shortenUrl_);
                Buffer += _T("\" border=0>");
            }
            else HTML_Link(Buffer, UrlList[i]);
            Buffer+=_T("<br/>&nbsp;<br/>");
        }
    }

    if(type == 7)
    {
        for(int i=0;i<n;i++)
        {
            if (UrlList[i].isNull())
            {
                continue;
            }
            HTML_Link(Buffer, UrlList[i]);
        }
    }

    if(UseTemplate && TemplateFoot && m_Page!=2)
        Buffer+=TemplateFoot;

    
    UrlListCS.Unlock();
    return Buffer;
}

void CResultsPanel::UpdateOutput()
{
    CString code = GenerateOutput();
    SetDlgItemText(IDC_CODEEDIT,code);
}


LRESULT CResultsPanel::OnCbnSelchangeCodetype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    UpdateOutput();
    BOOL temp;

    if(Settings.AutoCopyToClipboard)
        OnBnClickedCopyall(0,0,0,temp);
     return 0;
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

LRESULT  CResultsPanel::OnEditChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    BOOL temp;
    OnCbnSelchangeCodetype(0, 0, 0, temp);
    return 0;
 }

LRESULT CResultsPanel::OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if(!*WizardDlg->LastVideoFile) return 0;
    CMediaInfoDlg dlg ;
    dlg.ShowInfo(WizardDlg->LastVideoFile);
    return 0;
    
}
    
LRESULT CResultsPanel::OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{    
    LogWindow.Show();
    return 0;
}

int CResultsPanel::GetCodeType()
{
    return  SendDlgItemMessage(IDC_CODETYPE,CB_GETCURSEL);
}

void CResultsPanel::SetCodeType(int Index)
{
    SendDlgItemMessage(IDC_CODETYPE, CB_SETCURSEL, Index);
    BOOL temp;
    OnCbnSelchangeCodetype(0, 0, 0, temp); //FIXME
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
    CString XmlFileName = IuCommonFunctions::GetDataFolder() + _T("templates.xml");
    LoadTemplateFromFile(XmlFileName, Error);
    CString userTemplateError;
    CString userTemplateFile = Utf8ToWCstring(Settings.SettingsFolder) +  _T("user_templates.xml");
    LoadTemplateFromFile(userTemplateFile, userTemplateError);

    return true;
}

bool CResultsPanel::LoadTemplateFromFile(const CString fileName, CString &Error)
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
    NMTOOLBAR* pnmtb = reinterpret_cast<NMTOOLBAR *>(pnmh);
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
        if ( OnShortenUrlChanged ) {
            menuItemTitle.Format(TR("Shorten URL using %s"), IuCoreUtils::Utf8ToWstring(Settings.urlShorteningServer.serverName()).c_str());
        } else {
            menuItemTitle.Format(TR("Shorten URL"));
        }
        mi.dwTypeData  = (LPWSTR)(LPCTSTR)menuItemTitle;
        sub.InsertMenuItem(count++, true, &mi);
    

    mi.fType = MFT_STRING;
    mi.wID = IDC_USEDIRECTLINKS;
    mi.dwTypeData = TR_CONST("Use direct links");//TR("Autorization parameters");
    sub.InsertMenuItem(count++, true, &mi);

/*    mi.fType = MFT_STRING;
    mi.wID = IDC_USEDIRECTLINKS;
    mi.dwTypeData  = TR("Open links in new tab (HTML only)");//TR("Autorization parameters");
    sub.InsertMenuItem(count++, true, &mi);*/


    mi.wID = IDC_USETEMPLATE;
    mi.dwTypeData = TR_CONST("Use template");
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
        mi.dwTypeData  = (LPWSTR)(LPCTSTR) title;//TR("Autorization parameters");
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
    
    sub.CheckMenuItem(IDC_USEDIRECTLINKS, MF_BYCOMMAND    | (Settings.UseDirectLinks? MF_CHECKED    : MF_UNCHECKED)    );
    sub.CheckMenuItem(IDC_USETEMPLATE, MF_BYCOMMAND    | (Settings.UseTxtTemplate? MF_CHECKED    : MF_UNCHECKED)    );        
    sub.CheckMenuItem(IDC_SHORTENURLITEM, MF_BYCOMMAND    | (shortenUrl_? MF_CHECKED    : MF_UNCHECKED)    );    
    
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
    Settings.UseTxtTemplate = !Settings.UseTxtTemplate;
    UpdateOutput();
    return 0;
}

LRESULT CResultsPanel::OnUseDirectLinksClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    Settings.UseDirectLinks = !Settings.UseDirectLinks;
    UpdateOutput();
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

void CResultsPanel::AddServer(ServerProfile server)
{
    for(size_t i=0; i<m_Servers.size(); i++)
        if (m_Servers[i].serverName() == server.serverName() 
            && 
            m_Servers[i].profileName() == server.profileName()
            && m_Servers[i].folderId() == server.folderId()
            )
            return;
    m_Servers.push_back(server);
    //return 0;
}
LRESULT CResultsPanel::OnResulttoolbarNMCustomDraw(LPNMHDR pnmh)
{
    LPNMTBCUSTOMDRAW lpNMCustomDraw = reinterpret_cast<LPNMTBCUSTOMDRAW>(pnmh);
    HDC dc = lpNMCustomDraw->nmcd.hdc;
    RECT toolbarRect = lpNMCustomDraw->nmcd.rc;
    //HTHEME hTheme = OpenThemeData(m_hWnd, _T("TAB"));
    RECT rc;
    GetClientRect(&rc);

    HMODULE hinstDll;

    // Check if the application is themed

    bool m_bThemeActive = false;
    hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
    if (hinstDll)
    {
        typedef BOOL (*ISAPPTHEMEDPROC)();
        ISAPPTHEMEDPROC pIsAppThemed;
        pIsAppThemed = reinterpret_cast<ISAPPTHEMEDPROC>(::GetProcAddress(hinstDll, "IsAppThemed"));

        if(pIsAppThemed)
            m_bThemeActive = pIsAppThemed() != FALSE;

        ::FreeLibrary(hinstDll);
    }
    if (m_bThemeActive) {
        //rc.top-=10;

        //m_wndTab.GetWindowRect(&rc);

        // Get the tab control DC

        //  HDC hDC = m_wndTab.GetDC();

        // Create a compatible DC

        HDC hDCMem = ::CreateCompatibleDC(dc);
        HBITMAP hBmp = ::CreateCompatibleBitmap(dc, rc.right - rc.left, rc.bottom - rc.top);
        HBITMAP hBmpOld = reinterpret_cast<HBITMAP>(::SelectObject(hDCMem, hBmp));

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

void CResultsPanel::setUrlList(CAtlArray<CUrlListItem>  * urlList)
{
    //UrlList = urlList;
}

LRESULT CResultsPanel::OnShortenUrlClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    shortenUrl_ = !shortenUrl_;
    if ( OnShortenUrlChanged ) {
        OnShortenUrlChanged(shortenUrl_);
    }
    return 0;
}


LRESULT CResultsPanel::OnPreviewButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    
    CString url ;
    if ( m_Page == 2 && this->UrlList.size() ) {
        //use
        url = this->UrlList[0].getImageUrl();
        if ((!Settings.UseDirectLinks || url.IsEmpty()) &&  !this->UrlList[0].getDownloadUrl().IsEmpty()) {
            url = this->UrlList[0].getDownloadUrl();
        }
    } else {
        CString outputTempFileName = IuCommonFunctions::IUTempFolder  + "preview.html";
        CString code = GenerateOutput();
        //ShowVar(m_Page);
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
        webViewWindow_ = new CWebViewWindow();
        webViewWindow_->Create(0,r,TR("Preview Window"),WS_POPUP|WS_OVERLAPPEDWINDOW,WS_EX_TOPMOST    );
        webViewWindow_->CenterWindow(WizardDlg->m_hWnd);
        webViewWindow_->ShowWindow(SW_SHOW);
    }
    
    webViewWindow_->NavigateTo(url);
    webViewWindow_->ShowWindow(SW_SHOW);
//    webViewWindow_->ActivateWindow();
    return 0;
}
