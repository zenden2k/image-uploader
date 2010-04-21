/*
    Image Uploader - application for uploading images/files to Internet
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

#include "stdafx.h"

#include "ResultsPanel.h"
#include <uxtheme.h>
#include "Common/regexp.h"
#include "mediainfodlg.h"

// CResultsPanel
CResultsPanel::CResultsPanel(CAtlArray<CUrlListItem> &p,CWizardDlg *dlg):UrlList(p),WizardDlg(dlg)
{
	m_nImgServer = m_nFileServer = -1;
	TemplateHead = TemplateFoot = NULL;	
	CString TemplateLoadError;
	if(!LoadTemplates(TemplateLoadError))
	{
		WriteLog(logWarning, _T("Results Module"), TemplateLoadError);
	}
}

CResultsPanel::~CResultsPanel()
{
	if(TemplateHead) delete[] TemplateHead;
}
	
bool CResultsPanel::LoadTemplate()
{
	if(TemplateHead && TemplateFoot) return true;

	DWORD dwBytesRead, dwFileSize;
	CString FileName = IU_GetDataFolder() + _T("template.txt");

	
	if(TemplateHead) delete[] TemplateHead;
	TemplateHead = NULL;
	TemplateFoot = NULL;
	HANDLE hFile = CreateFile(FileName,	GENERIC_READ	,0,0,OPEN_EXISTING	,0,0);
	if(!hFile) return false;

	dwFileSize = GetFileSize (hFile, NULL) ; 
	if(!dwFileSize) return false;
	DWORD dwMemoryNeeded = min(35536, dwFileSize);
	LPTSTR TemplateText = (LPTSTR) new CHAR[dwMemoryNeeded+2]; 
	if(!TemplateText) return false;
	ZeroMemory(TemplateText,dwMemoryNeeded);
	::ReadFile(hFile, (LPVOID)TemplateText , 2, &dwBytesRead, NULL); //Reading BOM
	if (::ReadFile(hFile, (LPVOID)TemplateText , dwFileSize, &dwBytesRead, NULL) == FALSE)
		return false;
	
	TemplateHead = TemplateText;

	LPTSTR szStart = (LPTSTR) wcsstr(TemplateText , _T("%images%"));
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
	TRC(IDC_IMAGEUPLOADERLABEL, "Картинок в строке:");
	TRC(IDC_CODETYPELABEL, "Тип кода:");
	TabBackgroundFix(m_hWnd);
	HBITMAP hBitmap;
	// Get color depth (minimum requirement is 32-bits for alpha blended images).
	int iBitsPixel = GetDeviceCaps(::GetDC(HWND_DESKTOP),BITSPIXEL);
	HIMAGELIST m_hToolBarImageList;
	if (iBitsPixel >= 32)
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

	RECT rc = {0,0,400,30};
	GetClientRect(&rc);
	rc.top = rc.bottom - 24;
	rc.left = 10;
	Toolbar.Create(m_hWnd,rc,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD | TBSTYLE_LIST |TBSTYLE_FLAT| CCS_NORESIZE|CCS_BOTTOM |/*CCS_ADJUSTABLE|*/CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
	
	
	Toolbar.SetButtonStructSize();
	Toolbar.SetButtonSize(30,18);
	Toolbar.SetImageList(m_hToolBarImageList);
	Toolbar.AddButton(IDC_COPYALL, TBSTYLE_BUTTON|BTNS_AUTOSIZE ,TBSTATE_ENABLED, 0, TR("Копировать в буфер"), 0);
	
	bool IsLastVideo = false;

	Toolbar.AddButton(IDC_MEDIAFILEINFO, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("Инфо о последнем видео"), 0);
	Toolbar.AddButton(IDC_VIEWLOG, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, TR("Лог ошибок"), 0);
	Toolbar.AddButton(/*IDC_USETEMPLATE*/IDC_OPTIONSMENU, TBSTYLE_DROPDOWN |BTNS_AUTOSIZE, TBSTATE_ENABLED, 3, TR("Опции"), 0);
	
	if(!IsLastVideo) 
		Toolbar.HideButton(IDC_MEDIAFILEINFO);
		
	Toolbar.AutoSize();
	Toolbar.SetWindowLong(GWL_ID, IDC_RESULTSTOOLBAR);

	SetDlgItemInt(IDC_THUMBSPERLINE, Settings.ThumbsPerLine);
	SendDlgItemMessage(IDC_THUMBPERLINESPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)0) );
	SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(TR("Таблица эскизов с увеличением по клику")));
	SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(TR("Эскизы с увеличением по клику")));
	SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(TR("Изображения")));
	SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(TR("Ссылки на картинки/файлы")));	
		
	for(int i=0;i<Templates.GetCount(); i++)
	{
		SendDlgItemMessage(IDC_CODETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)Templates[i].Name);	
	}
	
	SendDlgItemMessage(IDC_CODETYPE,CB_SETCURSEL, 0);
	LoadTemplate();
	
	return 1;  // Let the system set the focus
}

void CResultsPanel::SetPage(int Index)
{
	::EnableWindow(GetDlgItem(IDC_CODETYPELABEL), Index!=2);
	::EnableWindow(GetDlgItem(IDC_CODETYPE), Index!=2);
	::EnableWindow(GetDlgItem(IDC_IMAGEUPLOADERLABEL), Index!=2);
	
	::EnableWindow(GetDlgItem(	IDC_IMAGESPERLINELABEL), Index!=2);
	::EnableWindow(GetDlgItem(IDC_THUMBSPERLINE), Index!=2);
	::EnableWindow(GetDlgItem(IDC_THUMBPERLINESPIN), Index!=2);
	
	m_Page = Index;
	GenerateOutput();
}

void BBCode_Link(CString &Buffer, CUrlListItem &item)
{
	Buffer += _T("[url=");
	if(*item.ImageUrl && (Settings.UseDirectLinks || item.DownloadUrl.IsEmpty()))
		Buffer += item.ImageUrl;
	else 
		Buffer += item.DownloadUrl;
	Buffer += _T("]");
	Buffer += myExtractFileName(item.FileName);
	Buffer += _T("[/url]");

}

void HTML_Link(CString &Buffer, CUrlListItem &item)
{
	Buffer += _T("<a href=\"");
	if(*item.ImageUrl && (Settings.UseDirectLinks || item.DownloadUrl.IsEmpty()))
		Buffer += item.ImageUrl;
	else 
		Buffer += item.DownloadUrl;
	Buffer += _T("\">");
	Buffer += myExtractFileName(item.FileName);
	Buffer += _T("</a>");
}

void CResultsPanel::GenerateOutput()
{
	CString Buffer;
	if(!Toolbar.m_hWnd) return ;
	int Index =	GetCodeType();

	int type=0;

	if(m_Page==0)
		type=Index;
	if (m_Page==1) 
		type= 4+Index;
	if(m_Page==2)
		type=8;

	UrlListCS.Lock();

	int n=UrlList.GetCount();
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
			LPCTSTR fname=myExtractFileName(UrlList[i].FileName);

			m_Vars[_T("DownloadUrl")]=UrlList[i].DownloadUrl;
			m_Vars[_T("ImageUrl")]=UrlList[i].ImageUrl;
			m_Vars[_T("ThumbUrl")]=UrlList[i].ThumbUrl;
			m_Vars[_T("FileName")]=fname;
			m_Vars[_T("FullFileName")]=UrlList[i].FileName;
			m_Vars[_T("Index")]=IntToStr(i);
			TCHAR buffer[600];
			GetOnlyFileName(UrlList[i].FileName,buffer);
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
		SetDlgItemText(IDC_CODEEDIT,Buffer);
		UrlListCS.Unlock();
		return;
	}

	if(p<1) p=4;

	if(type==0||type==1)
	{
		for(int i=0;i<n;i++)
		{

			Buffer+=_T("[url=");
			if(*UrlList[i].ImageUrl&& (Settings.UseDirectLinks || UrlList[i].DownloadUrl.IsEmpty()))

				Buffer+=UrlList[i].ImageUrl;
			else 
				Buffer+=UrlList[i].DownloadUrl;
			Buffer+=_T("]");

			if(lstrlen(UrlList[i].ThumbUrl)>0)
			{
				Buffer+=_T("[img]");
				Buffer+=UrlList[i].ThumbUrl;
				Buffer+=_T("[/img]");
			}
			else
			{
				Buffer+=myExtractFileName(UrlList[i].FileName);
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
			if(*UrlList[i].ImageUrl&& (Settings.UseDirectLinks || UrlList[i].DownloadUrl.IsEmpty()))
			{
				Buffer+=_T("[img]");
				Buffer+=UrlList[i].ImageUrl;
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
			BBCode_Link(Buffer, UrlList[i]);

			Buffer+=_T("\r\n");
		}
	}
	if(type==8)
	{
		for(int i=0;i<n;i++)
		{
			if(*UrlList[i].ImageUrl&& (Settings.UseDirectLinks || UrlList[i].DownloadUrl.IsEmpty())) 
				Buffer+=UrlList[i].ImageUrl;
			else 
				Buffer+=UrlList[i].DownloadUrl;
			Buffer+=_T("\r\n");
		}
	}

	if(type==4  || type==5)
	{
		Buffer+=_T("<center>");
		for(int i=0;i<n;i++)
		{
			Buffer+=_T("<a href=\"");
			if(*UrlList[i].ImageUrl&& (Settings.UseDirectLinks || UrlList[i].DownloadUrl.IsEmpty())) 
				Buffer+=UrlList[i].ImageUrl;
			else 
				Buffer+=UrlList[i].DownloadUrl;
			Buffer+=_T("\">");
			if(lstrlen(UrlList[i].ThumbUrl)>0)
			{
				Buffer+=_T("<img src=\"");
				Buffer+=UrlList[i].ThumbUrl;
				Buffer+=_T("\" border=0>");
			}
			else
				Buffer+=myExtractFileName(UrlList[i].FileName);
			Buffer+=_T("</a>");
			if(((i+1)%p)&&type==4)
				Buffer+=_T("&nbsp;&nbsp;");
			if(!((i+1)%p) &&type==4||type==5 )
				Buffer+=_T("<br/>&nbsp;<br/>");
		}
		Buffer+=_T("</center>");
	}

	if(type == 6)
	{
		for(int i=0; i<n; i++)
		{
			if(lstrlen(UrlList[i].ImageUrl)>0 && (Settings.UseDirectLinks || UrlList[i].DownloadUrl.IsEmpty()))
			{
				Buffer += _T("<img src=\"");
				Buffer += UrlList[i].ImageUrl;
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
			HTML_Link(Buffer, UrlList[i]);
		}
	}

	if(UseTemplate && TemplateFoot && m_Page!=2)
		Buffer+=TemplateFoot;

	SetDlgItemText(IDC_CODEEDIT,Buffer);
	UrlListCS.Unlock();
}

LRESULT CResultsPanel::OnCbnSelchangeCodetype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GenerateOutput();
	BOOL temp;

	if(Settings.AutoCopyToClipboard)
		OnBnClickedCopyall(0,0,0,temp);
	 return 0;
 }

LRESULT CResultsPanel::OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SendDlgItemMessage(IDC_CODEEDIT, EM_SETSEL, 0, -1);
	SendDlgItemMessage(IDC_CODEEDIT, WM_COPY, 0, 0);
	SendDlgItemMessage(IDC_CODEEDIT, EM_SETSEL, -1, 0);
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
	int i =0;

	CMarkupMSXML XML;
	CString XmlFileName = IU_GetDataFolder() + _T("templates.xml");

	if(!FileExists(XmlFileName))
	{
		Error = TR("Файл не найден.");
		return false;
	}

	if(!XML.Load(XmlFileName))
	{
		Error = XML.GetError();
		return false;
	}

	if(!XML.FindElem(_T("Templates")))
	{
		Error = _T("Unable to find Templates node");
		return false;
	}; 

	XML.IntoElem();

	while(XML.FindElem(_T("Template")))
	{
		IU_Result_Template Template;
		Template.Name = XML.GetAttrib(_T("Name"));
		
		XML.IntoElem();
		
		if(XML.FindElem(_T("Text")))
		{
			Template.TemplateText = XML.GetData();
		}

		if(XML.FindElem(_T("Items")))
		{
			Template.LineStart = XML.GetAttrib(_T("LineStart"));
			Template.LineEnd = XML.GetAttrib(_T("LineEnd"));
			Template.LineSep = XML.GetAttrib(_T("LineSep"));
			Template.ItemSep = XML.GetAttrib(_T("ItemSep"));
			
			Template.Items = XML.GetData();
		}
		XML.OutOfElem();

		Templates.Add(Template);
	}
	return true;
}

CString CResultsPanel::ReplaceVars(const CString Text)
{
	CString Result;
	Result =  Text;

	RegExp exp;
	CComBSTR Pat = _T("\\$\\(([A-z_]*?)\\)");
	CComBSTR Txt = Text;
	exp.SetPattern(Pat);
	exp.Execute(Txt);

	int n = exp.MatchCount();

	for(int i=0; i<n; i++) 
	{
		CComBSTR VarName = exp.GetSubMatch(i, 0);
		CString vv = VarName;
		Result.Replace(CString(_T("$(")) + vv + _T(")"), m_Vars[vv]);
	}

	Result.Replace(_T("\\n"), _T("\r\n"));
	return Result;
}

LRESULT CResultsPanel::OnOptionsDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMTOOLBAR* pnmtb = (NMTOOLBAR *) pnmh;
	CMenu sub;	
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_TYPE|MIIM_ID;
	mi.fType = MFT_STRING;
	sub.CreatePopupMenu();
	RECT rc;
	
	::GetWindowRect(GetDlgItem(IDC_RESULTSTOOLBAR),&rc);
	POINT ScreenPoint ={rc.left,rc.top};
		
	std::map<int,int>::iterator it;
	int count = 0;

	for(it = m_Servers.begin(); it!= m_Servers.end(); it++)
	{
		UploadEngine &ue = EnginesList[it->first];
		CString folderTitle = Settings.ServersSettings[ue.Name].params[_T("FolderTitle")];
		CString folderUrl = Settings.ServersSettings[ue.Name].params[_T("FolderUrl")];

		if(folderTitle.IsEmpty() || folderUrl.IsEmpty()) continue;
		CString title = TR("Копировать URL адрес ") + CString(ue.Name)+ _T("->")+folderTitle;
		mi.wID = IDC_COPYFOLDERURL + it->first;
		mi.dwTypeData  = (LPWSTR)(LPCTSTR) title;//TR("Параметры авторизации");
		sub.InsertMenuItem(count++, true, &mi);
	}
	if(count)
	{
		mi.wID = IDC_FILESERVER_LAST_ID + 1;
		mi.fType = MFT_SEPARATOR;
		sub.InsertMenuItem(count, true, &mi);
		count++;
	}
	
	mi.fType = MFT_STRING;
	mi.wID = IDC_USEDIRECTLINKS;
	mi.dwTypeData  = TR("Использовать прямые ссылки");//TR("Параметры авторизации");
	sub.InsertMenuItem(count++, true, &mi);

	mi.wID = IDC_USETEMPLATE;
 	mi.dwTypeData  = TR("Использовать шаблон");
	sub.InsertMenuItem(count++, true, &mi);


	
			
	sub.CheckMenuItem(IDC_USEDIRECTLINKS, MF_BYCOMMAND	| (Settings.UseDirectLinks? MF_CHECKED	: MF_UNCHECKED)	);
	sub.CheckMenuItem(IDC_USETEMPLATE, MF_BYCOMMAND	| (Settings.UseTxtTemplate? MF_CHECKED	: MF_UNCHECKED)	);
		


		
   ::SendMessage(Toolbar.m_hWnd,TB_GETRECT, pnmtb->iItem, (LPARAM)&rc);
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
	GenerateOutput();
	return 0;
}

LRESULT CResultsPanel::OnUseDirectLinksClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Settings.UseDirectLinks = !Settings.UseDirectLinks;
	GenerateOutput();
	return 0;
}

LRESULT CResultsPanel::OnCopyFolderUrlClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int ServerID = wID - IDC_COPYFOLDERURL;

	UploadEngine &ue = EnginesList[ServerID];
CString folderUrl = Settings.ServersSettings[ue.Name].params[_T("FolderUrl")];

	IU_CopyTextToClipboard(folderUrl);
	return 0;
}

void CResultsPanel::AddServerId(int serverId)
{
	
	m_Servers[serverId] = serverId;
	//return 0;
}