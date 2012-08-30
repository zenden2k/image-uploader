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

#ifndef COMMON_H
#define COMMON_H

#include "atlheaders.h"
#include <ctime>
#include "PluginLoader.h"
#include "MyEngineList.h"
#include <GdiPlus.h>
#include "CommonTypes.h"

#define IU_NEWFOLDERMARK ("_iu_create_folder_")
#define APPNAME _T("Image Uploader")
#define IMAGE_DIALOG_FORMATS _T("Image files (JPEG, GIF, PNG, etc)\0*.jpg;*.gif;*.png;*.bmp;*.tiff\0All files\0*.*\0\0")

class CPluginManager;
class CCmdLine;

bool IULaunchCopy();
BOOL CreateTempFolder();
void ClearTempFolder(LPCTSTR folder);
extern CString IUTempFolder;

extern CString IUCommonTempFolder;
extern CCmdLine CmdLine;

bool IULaunchCopy(CString params, CAtlArray<CString> &files);
bool IULaunchCopy(CString additionalParams=_T(""));

extern CPluginManager iuPluginManager;

void IU_ConfigureProxy(NetworkManager& znm);

const CString IU_GetVersion();

void IU_RunElevated(CString params);


bool IsImage(LPCTSTR szFileName);
bool IsVideoFile(LPCTSTR szFileName);
CString GenerateFileName(const CString &templateStr, int index,const CPoint size, const CString& originalName=_T(""));
extern CMyEngineList *_EngineList;

const CString IU_GetDataFolder();
Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance,LPCTSTR szResName, LPCTSTR szResType);
void DecodeString(LPCTSTR szSource, CString &Result, LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ");
void EncodeString(LPCTSTR szSource, CString &Result,LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ");

#endif