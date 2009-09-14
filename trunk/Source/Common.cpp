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

