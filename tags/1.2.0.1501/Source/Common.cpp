/*
    Image Uploader - program for uploading images/files to Internet
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
#include "Common.h"

CString IUCommonTempFolder, IUTempFolder;
CCmdLine CmdLine;
CAppModule _Module;

CWizardPage::~CWizardPage()
{
}

bool CWizardPage::OnShow()
{
	EnableNext();
	EnablePrev();
	ShowNext();
	ShowPrev();

	return true;
}
bool CWizardPage:: OnNext()
{
	return true;
}

void CWizardPage::EnableNext(bool Enable)
{
	if(!WizardDlg) return;
	::EnableWindow(WizardDlg->GetDlgItem(IDC_NEXT),Enable);
}
void CWizardPage::EnablePrev(bool Enable)
{
	if(!WizardDlg) return;
	::EnableWindow(WizardDlg->GetDlgItem(IDC_PREV),Enable);
}
void CWizardPage::EnableExit(bool Enable)
{
	if(!WizardDlg) return;
	::EnableWindow(WizardDlg->GetDlgItem(IDCANCEL),Enable);
}

void CWizardPage::SetNextCaption(LPTSTR Caption)
{
	if(!WizardDlg) return;
	WizardDlg->SetDlgItemText(IDC_NEXT,Caption);
}
void CWizardPage::ShowNext(bool Show)
{
	if(!WizardDlg) return;
	::ShowWindow(WizardDlg->GetDlgItem(IDC_NEXT),Show?SW_SHOW:SW_HIDE);
}
void CWizardPage::ShowPrev(bool Show)
{
	if(!WizardDlg) return;
	::ShowWindow(WizardDlg->GetDlgItem(IDC_PREV),Show?SW_SHOW:SW_HIDE);
}
bool CWizardPage:: OnHide()
{
	return false;
}

int GetUploadEngineIndex(const CString Name)
{
	for(int i=0; i<EnginesList.GetCount(); i++)
	{
		if(EnginesList[i].Name == Name) return i;
	}
	return 0;
}

	WIN32_FIND_DATA wfd;
	HANDLE findfile = 0;

int GetNextImgFile(LPTSTR szBuffer, int nLength)
{
	TCHAR szNameBuffer[MAX_PATH], szBuffer2[MAX_PATH], TempPath[256];
	
	GetTempPath(256, TempPath);
	wsprintf(szBuffer2, _T("%s*.*"), (LPCTSTR)IUTempFolder);
	
	if(!findfile)
	{
		findfile = FindFirstFile(szBuffer2, &wfd);
		if(!findfile) goto error;
	}
	else 
	{
		if(!FindNextFile(findfile, &wfd))
			goto error;

	}
	if(lstrlen(wfd.cFileName) < 1) goto error;
	lstrcpyn(szBuffer, wfd.cFileName, nLength);

	return TRUE;

error:
	if(findfile) FindClose(findfile);
	return FALSE;
}

void DeleteDir2(LPCTSTR Dir)
{
	if(!Dir) return;
	TCHAR szBuffer[MAX_PATH];
	lstrcpyn(szBuffer, Dir, MAX_PATH);
	int nLen = lstrlen(szBuffer)-1;
	if(szBuffer[nLen] == _T('\\')) szBuffer[nLen] = 0;

	SHFILEOPSTRUCT FileOp;
	ZeroMemory(&FileOp, sizeof(FileOp));
	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE;
	FileOp.pFrom = szBuffer;
	FileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT|FOF_NOERRORUI;
	SHFileOperation(&FileOp);
}

void ClearTempFolder()
{
	TCHAR szBuffer[256] = _T("\0");
	TCHAR szNameBuffer[MAX_PATH], szBuffer2[MAX_PATH], TempPath[256];
	GetTempPath(256, TempPath);
	
	while(GetNextImgFile(szBuffer, 256))
	{
		#ifdef DEBUG
			if(!lstrcmpi(szBuffer, _T("log.txt"))) continue;
		#endif
		wsprintf(szBuffer2,_T("%s%s"), (LPCTSTR) IUTempFolder, szBuffer);
		DeleteFile(szBuffer2);
	}
	if(!RemoveDirectory(IUTempFolder))
	{
		DeleteDir2(IUTempFolder);
	}
}

bool IULaunchCopy()
{
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
	
	ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);				 
   ZeroMemory(&pi, sizeof(pi));

	TCHAR Buffer[MAX_PATH*40];
	GetModuleFileName(0, Buffer, sizeof(Buffer)/sizeof(TCHAR));

	CString TempCmdLine = CString(_T("\""))+Buffer+CString(_T("\"")); 
	for(int i=1;i <CmdLine.GetCount(); i++)
		{
			if(!lstrcmpi(CmdLine[i], _T("-Embedding"))) continue;
			TempCmdLine = TempCmdLine + " \"" + CmdLine[i] + "\""; 
		}

    // Start the child process.
    if( !CreateProcess(
		NULL,                   // No module name (use command line). 
        (LPWSTR)(LPCTSTR)TempCmdLine, // Command line. 
        NULL,                   // Process handle not inheritable. 
        NULL,                   // Thread handle not inheritable. 
        FALSE,                  // Set handle inheritance to FALSE. 
        0,                      // No creation flags. 
        NULL,                   // Use parent's environment block. 
        NULL,                   // Use parent's starting directory. 
        &si,                    // Pointer to STARTUPINFO structure.
        &pi )                   // Pointer to PROCESS_INFORMATION structure.
    ) 
    
        return false;

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	return true;
}


BOOL CreateTempFolder()
{
	TCHAR TempPath[256];
	GetTempPath(256, TempPath);
	DWORD pid = GetCurrentProcessId() ^ 0xa1234568;
	IUCommonTempFolder.Format(_T("%stmd_iu_temp"), TempPath);
	
	CreateDirectory(IUCommonTempFolder,0);
	IUTempFolder.Format(_T("%s\\iu_temp_%x"),(LPCTSTR) IUCommonTempFolder, pid);
	
	CreateDirectory(IUTempFolder,0);

	IUTempFolder+=_T("\\");
	return TRUE;
}

bool MySaveImage(Image *img,LPTSTR szFilename,LPTSTR szBuffer,int Format,int Quality)
{
	if(Format==-1) Format=0;
	TCHAR szImgTypes[3][4]={_T("jpg"),_T("png"),_T("gif")};
	TCHAR szMimeTypes[3][12]={_T("image/jpeg"),_T("image/png"),_T("image/gif")};
	TCHAR szNameBuffer[MAX_PATH],szBuffer2[MAX_PATH],TempPath[256];
	GetOnlyFileName(szFilename,szNameBuffer);
	wsprintf(szBuffer2,_T("%s%s%d.%s"),IUTempFolder,szNameBuffer,(int)GetTickCount(),szImgTypes[Format]);
	
	CLSID clsidEncoder;
	EncoderParameters eps;
	eps.Count = 1;
	eps.Parameter[0].Guid = EncoderQuality;
	eps.Parameter[0].Type = EncoderParameterValueTypeLong;
	eps.Parameter[0].NumberOfValues = 1;
	eps.Parameter[0].Value = &Quality;

	if(GetEncoderClsid(szMimeTypes[Format], &clsidEncoder)!=-1)
	{
		if(Format == 0)
			img->Save(szBuffer2,&clsidEncoder,&eps);
		img->Save(szBuffer2,&clsidEncoder);
	}
	lstrcpy(szBuffer,szBuffer2);

	return true;
}

#define HOTKEY(modifier,key) ((((modifier)&0xff)<<8)|((key)&0xff)) 

// Создание ярлыка 
// Входные параметры: 
//  pwzShortCutFileName - путь и имя ярлыка, например, "C:\\Блокнот.lnk" 
//  Если не указан путь, ярлык будет создан в папке, указанной в следующем параметре. 
//  Прим.: Windows сама НЕ добавляет к имени расширение .lnk 
//  pszPathAndFileName  - путь и имя exe-файла, например, "C:\\Windows\\NotePad.Exe" 
//  pszWorkingDirectory - рабочий каталог, например, "C:\\Windows" 
//  pszArguments        - аргументы командной строки, например, "C:\\Doc\\Text.Txt" 
//  wHotKey             - горячая клавиша, например, для Ctrl+Alt+A     HOTKEY(HOTKEYF_ALT|HOTKEYF_CONTROL,'A') 
//  iCmdShow            - начальный вид, например, SW_SHOWNORMAL 
//  pszIconFileName     - путь и имя файла, содержащего иконку, например, "C:\\Windows\\NotePad.Exe" 
//  int iIconIndex      - индекс иконки в файле, нумеруется с 0 
bool __fastcall CreateShortCut( 
							   LPCWSTR pwzShortCutFileName, 
							   LPCTSTR pszPathAndFileName, 
							   LPCTSTR pszWorkingDirectory, 
							   LPCTSTR pszArguments, 
							   WORD wHotKey, 
							   int iCmdShow, 
							   LPCTSTR pszIconFileName, 
							   int iIconIndex) 
{ 
					   IShellLink * pSL; 
	IPersistFile * pPF; 
	HRESULT hRes; 
	if( CoInitialize(NULL) != S_OK);
		//return false;
	// Получение экземпляра компонента "Ярлык" 
	hRes = CoCreateInstance(CLSID_ShellLink, 0,	CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pSL); 

	if( SUCCEEDED(hRes) ) 
	{ 
		hRes = pSL->SetPath(pszPathAndFileName); 
		if( SUCCEEDED(hRes) ) 
		{ 
			hRes = pSL->SetArguments(pszArguments); 
			//if( SUCCEEDED(hRes) ) 
			{ 
				hRes = pSL->SetWorkingDirectory(pszWorkingDirectory); 
				if( SUCCEEDED(hRes) ) 
				{ 
					hRes = pSL->SetIconLocation(pszIconFileName,iIconIndex); 
					if( SUCCEEDED(hRes) ) 
					{ 
					//	hRes = pSL->SetHotkey(wHotKey); 
					//	if( SUCCEEDED(hRes) ) 
						{ 
							hRes = pSL->SetShowCmd(iCmdShow); 
							if( SUCCEEDED(hRes) ) 
							{ 
								// Получение компонента хранилища параметров 
								hRes = pSL->QueryInterface(IID_IPersistFile,(LPVOID *)&pPF); 
								if( SUCCEEDED(hRes) ) 
								{ 
									// Сохранение созданного ярлыка 
									hRes = pPF->Save(pwzShortCutFileName,TRUE); 
									pPF->Release(); 
								} 
							} 
						} 
					} 
				} 
			} 
		} 
		pSL->Release(); 
	} 
	return SUCCEEDED(hRes); 

}  

void DrawStrokedText(Graphics &gr, LPCTSTR Text,RectF Bounds,Font &font,Color &ColorText,Color &ColorStroke,int HorPos,int VertPos, int width)
{
	RectF OriginalTextRect, NewTextRect;
	FontFamily ff;
	font.GetFamily(&ff);
	gr.SetPageUnit(UnitPixel);
	gr.MeasureString(Text,-1,&font,PointF(0,0),&OriginalTextRect);

	Font NewFont(&ff,48,font.GetStyle(),UnitPixel);
	gr.MeasureString(Text,-1,&NewFont,RectF(0,0,5000,1600),&NewTextRect);
	OriginalTextRect.Height = OriginalTextRect.Height-OriginalTextRect.Y;
	float newwidth,newheight;
	newheight = OriginalTextRect.Height;
	newwidth=OriginalTextRect.Height/NewTextRect.Height*NewTextRect.Width;
	float k = 2*width*NewTextRect.Height/OriginalTextRect.Height;
	SolidBrush br(ColorText);
	Bitmap temp(NewTextRect.Width,NewTextRect.Height,&gr);

	Graphics gr_temp(&temp);
	StringFormat format;
	gr_temp.SetPageUnit(UnitPixel);
	GraphicsPath path;
	gr_temp.SetSmoothingMode( SmoothingModeHighQuality);
	path.AddString(Text, -1,&ff, (int)NewFont.GetStyle(), NewFont.GetSize(), Point(0,0), &format);

	Pen pen(ColorStroke,(float)k);
	pen.SetAlignment(PenAlignmentCenter);

	float x,y;
	gr_temp.DrawPath(&pen, &path);
	gr_temp.FillPath(&br, &path);
	gr.SetSmoothingMode( SmoothingModeHighQuality); 
	gr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );

	if(HorPos == 0)
		x = 2;
	else if(HorPos == 1)
		x = (Bounds.Width-newwidth)/2;
	else x=(Bounds.Width-newwidth)-2;

	if(VertPos==0)
		y=2;
	else if(VertPos==1)
		y=(Bounds.Height-newheight)/2;
	else y=(Bounds.Height-newheight)-2;
	 
	gr.DrawImage(&temp,(int)(Bounds.GetLeft()+x),(int)(Bounds.GetTop()+y),(int)(newwidth),(int)(newheight));
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}