/*
    Image Uploader - application for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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

	TCHAR szFileName[256], szPath[256];
	DWORD dwBytesRead, dwFileSize;
	GetModuleFileName(0, szFileName, 1023);
	ExtractFilePath(szFileName, szPath);
	wsprintf(szFileName, _T("%sData\\template.txt"), szPath);
	if(TemplateHead) delete[] TemplateHead;
	TemplateHead=NULL;
	TemplateFoot = NULL;
	HANDLE hFile = CreateFile(szFileName,	GENERIC_READ	,0,0,OPEN_EXISTING	,0,0);
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
		TemplateFoot=szStart+8;
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
	Toolbar.AddButton(IDC_USETEMPLATE, TBSTYLE_CHECK |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, TR("Использовать шаблон"), 0);
	Toolbar.AddButton(IDC_VIEWLOG, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 3, TR("Лог ошибок"), 0);
	
	if(!IsLastVideo) 
		Toolbar.HideButton(IDC_MEDIAFILEINFO);
		
	Toolbar.AutoSize();

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
		
	Toolbar.CheckButton(IDC_USETEMPLATE,Settings.UseTxtTemplate);
		
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

void BBCode_Link(CString &Buffer,CUrlListItem &item)
{
	Buffer += _T("[url=");
	if(*item.ImageUrl)
		Buffer += item.ImageUrl;
	else 
		Buffer += item.DownloadUrl;
	Buffer += _T("]");
	Buffer += myExtractFileName(item.FileName);
	Buffer += _T("[/url]");

}

void HTML_Link(CString &Buffer,CUrlListItem &item)
{
	Buffer += _T("<a href=\"");
	if(*item.ImageUrl)
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

	bool UseTemplate = Toolbar.IsButtonChecked(IDC_USETEMPLATE); //IsChecked(IDC_USETEMPLATE);
	Settings.UseTxtTemplate = UseTemplate;
	if(UseTemplate && TemplateHead && type!=4)
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
			if(*UrlList[i].ImageUrl)

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
			if(*UrlList[i].ImageUrl)
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
			if(*UrlList[i].ImageUrl) 
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
			if(*UrlList[i].ImageUrl) 
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
			if(lstrlen(UrlList[i].ImageUrl)>0)
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

	if(UseTemplate && TemplateFoot && type!=4)
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
	// Копирование текста в буффер обмена
	SendDlgItemMessage(IDC_CODEEDIT, EM_SETSEL, 0, -1);
	SendDlgItemMessage(IDC_CODEEDIT, WM_COPY, 0, 0);
	SendDlgItemMessage(IDC_CODEEDIT, EM_SETSEL, -1, 0);
	
	return 0;
}

LRESULT  CResultsPanel::OnEditChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// 
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
	CString XmlFileName = GetAppFolder()+_T("Data\\templates.xml");

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