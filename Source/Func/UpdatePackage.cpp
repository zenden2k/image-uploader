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

#include "../atlheaders.h"
#include "UpdatePackage.h"
#include "../3rdpart/unzipper.h"
#include "Common.h"
#include "pluginloader.h"
#include "../Gui/Dialogs/TextViewDlg.h"
#include "../Gui/Dialogs/LogWindow.h"
#include <time.h>

BOOL CreateFolder(LPCTSTR szFolder)
{
	if (!szFolder || !lstrlen(szFolder))
		return FALSE;

	DWORD dwAttrib = GetFileAttributes(szFolder);

	// already exists ?
	if (dwAttrib != 0xffffffff)
		return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

	// recursively create from the top down
	TCHAR* szPath = _tcsdup(szFolder);
	TCHAR* p = _tcsrchr(szPath, '\\');

	if (p) 
	{
		// The parent is a dir, not a drive
		*p = '\0';
			
		// if can't create parent
		if (!CreateFolder(szPath))
		{
			free(szPath);
			return FALSE;
		}
		free(szPath);

		if (!::CreateDirectory(szFolder, NULL)) 
			return FALSE;
	}
	
	return TRUE;
}

CUpdateInfo::CUpdateInfo()
{

}

bool CUpdateInfo::LoadUpdateFromFile(const CString& filename)
{
	ZSimpleXml xml;
	if(!xml.LoadFromFile(WCstringToUtf8(filename)))	
	{
		WriteLog(logError,_T("Update Engine"),CString(_T("Failed to load update file "))+filename+_T("\r\n"));
	}
	m_FileName = filename;
	Parse(xml);
	return true;
}

bool CUpdateInfo::SaveToFile(const CString& filename)
{
	FILE *f = _wfopen(filename, _T("wb"));
	if(!f) return false;
	
	std::string outbuf = WCstringToUtf8(m_Buffer);
	fwrite(outbuf.c_str(), 1, outbuf.size(), f);
	fclose(f);
	return false;
}

bool CUpdateInfo::LoadUpdateFromBuffer(const CString& buffer)
{
	ZSimpleXml m_xml;
	if(!m_xml.LoadFromString(WCstringToUtf8(buffer)))
	{
		WriteLog(logError,_T("Update Engine"),CString(_T("Failed to load update file \r\n"))+_T("\r\nServer answer:\r\n")+buffer);
		return false;
	}
	m_Buffer = buffer;
	
	Parse(m_xml);
	
	return true;
}

bool CUpdateInfo::DoUpdate(const CUpdateInfo &newPackage)
{
	//Comparing this package with newPackage and performing updates
	return true;
}

bool CUpdateInfo::Parse( ZSimpleXml &xml)
{
	ZSimpleXmlNode root = xml.getRoot("UpdateInfo", false);
	if(root.IsNull()) return false;
	
	CString packageName, UpdateUrl;
	m_PackageName = Utf8ToWCstring(root.Attribute("Name"));
	m_UpdateUrl = Utf8ToWCstring(root.Attribute("UpdateUrl"));
	m_DownloadUrl = Utf8ToWCstring(root.Attribute("DownloadUrl"));
	m_Hash = Utf8ToWCstring(root.Attribute("Hash"));
	m_TimeStamp = root.AttributeInt("TimeStamp");
	m_DisplayName = Utf8ToWCstring(root.Attribute("DisplayName"));
		
	/*m_xml.GetAttrib(_T("Name"), m_PackageName);
		m_xml.GetAttrib(_T("UpdateUrl"), m_UpdateUrl);
		m_xml.GetAttrib(_T("DownloadUrl"), m_DownloadUrl);
		m_xml.GetAttrib(_T("Hash"), m_Hash);
		m_xml.GetAttrib(_T("TimeStamp"), m_TimeStamp);
		m_xml.GetAttrib(_T("DisplayName"), m_DisplayName);*/

		if(m_PackageName.IsEmpty() || m_UpdateUrl.IsEmpty() || m_DownloadUrl.IsEmpty() || m_Hash.IsEmpty()  || !m_TimeStamp)
			return false;
		int core = 0;
		core=root.AttributeInt("CoreUpdate");
		m_CoreUpdate = core!=0;
		
		//if(m_xml.FindElem(_T("Info")))
	//	{
			m_ReadableText = Utf8ToWCstring(root["Info"].Text());
			//m_xml.GetData(m_ReadableText);
			m_ReadableText.Replace(_T("\n"),_T("\r\n"));
		//}
	
	return true;
}

bool CUpdateInfo::CanUpdate(const CUpdateInfo& newInfo)
{
	if(m_TimeStamp >= newInfo.m_TimeStamp) return false;
	if(newInfo.m_PackageName != m_PackageName) return false;
	return true;
}

bool CUpdateInfo::CheckUpdates()
{
	return true;
}

const CString CUpdateInfo::getHash()
{
	return m_Hash;
}

bool CUpdateInfo::operator< (const CUpdateInfo& p)
{
	return m_TimeStamp < p.m_TimeStamp;
}

bool CUpdateManager::CheckUpdates()
{
	m_ErrorStr.Empty();
	Clear();
	std::vector<CString> fileList;
	GetFolderFileList(fileList, IU_GetDataFolder() +_T("Update"),_T("*.xml"));

	bool Result = true;

	if(fileList.size() == 0)
	{
		m_ErrorStr = "No update files found in folder '" +IU_GetDataFolder() + _T("Update\\'");
		return false;
	}

	for(size_t i=0; i<fileList.size(); i++)
	{
		CString fileName = IU_GetDataFolder()+_T("Update\\") + fileList[i];
		if(!internal_load_update(fileName))
			Result= false;
	}

	//Result = m_updateList.size()!=0;
	return Result;
}

const CString CUpdateManager::ErrorString()
{
	return m_ErrorStr;
}

bool CUpdateManager::DoUpdates()
{
	for(size_t i=0; i<m_updateList.size(); i++)
	{
		nCurrentIndex = i;
		if(m_stop) return 0;
		internal_do_update(m_updateList[i]);
	}
	return true;
}

bool CUpdateManager::internal_load_update(CString name)
{
	
	CUpdateInfo localPackage;


	if(!localPackage.LoadUpdateFromFile(name))
	{
		WriteLog(logError,_T("Update Engine"),CString(TR("Невозможно загрузить файл обновления \'"))+name);
		return false;
	}

	CUpdateInfo remotePackage;
	NetworkManager nm;
	IU_ConfigureProxy(nm);	

	CString url = localPackage.m_UpdateUrl;
	url.Replace(_T("%appver%"), IU_GetVersion());
	url.Replace(_T("%name%"), localPackage.m_PackageName);

	nm.doGet(WCstringToUtf8( url));

	if(nm.responseCode() != 200)
	{
		WriteLog(logWarning,_T("Update Engine"), _T("Невозможно загрузить информацию о пакете обновления ") + localPackage.m_PackageName + CString(_T("\r\nHTTP response code: "))+IntToStr(nm.responseCode())+_T("\r\n")+ Utf8ToWstring(nm.errorString()).c_str(),CString("URL=")+url);		
		return false;
	}

	std::wstring res = Utf8ToWstring( nm.responseBody());
	if(!remotePackage.LoadUpdateFromBuffer(res.c_str()))
	{
		return false;
	}

	if(!localPackage.CanUpdate(remotePackage )) return true;
	remotePackage.m_FileName = localPackage.m_FileName;

	m_updateList.push_back(remotePackage);
	if(remotePackage.m_CoreUpdate) m_nCoreUpdates++;	
	return true;
}

bool CUpdateManager::AreCoreUpdates()
{
	return m_nCoreUpdates!=0;
}

bool CUpdateManager::internal_do_update(CUpdateInfo& ui)
{
	CString filename = IUTempFolder + ui.m_PackageName+_T(".zip");
	std::string filenamea= WCstringToUtf8(filename);
	nm.setOutputFile( filenamea);

	m_statusCallback->updateStatus(nCurrentIndex, TR("Скачивание файла ")+ ui.m_DownloadUrl);
	
	nm.doGet(WCstringToUtf8( ui.m_DownloadUrl));
	if(nm.responseCode() != 200)
	{
		WriteLog(logError,_T("Update Engine"),TR("Ошибка обновления компонента ") + ui.m_PackageName + CString(_T("\r\nHTTP response code: "))+IntToStr(nm.responseCode())+_T("\r\n")+ Utf8ToWstring(nm.errorString()).c_str(),CString("URL=")+ui.m_DownloadUrl);		
		return 0;
	}

	if( ui.getHash() != IU_md5_file(filename) || ui.getHash().IsEmpty())
	{
		updateStatus(0, CString(TR("Не совпал MD5 хэш пакета обновления "))+myExtractFileName( filename));
		return 0;
	}

	CUnzipper unzipper(filename);
	CString unzipFolder = IUTempFolder + ui.m_PackageName;
	if(!unzipper.UnzipTo(unzipFolder))
	{
		updateStatus(0, TR("Невозможно распаковать архив ")+ filename);
		return 0;
	}

	CUpdatePackage updatePackage;
	updatePackage.setUpdateStatusCallback(this);
	if(!updatePackage.LoadUpdateFromFile(unzipFolder + _T("\\")+_T("package.xml")))
	{
		MessageBox(0,TR("Не могу прочитать ") + ui.m_PackageName,0,0);
		return false;
	}

	if(!updatePackage.doUpdate())
		return false;
	CString finishText;
	finishText.Format(TR("Обновление завершено. Обновлено %d из %d файлов "), updatePackage.m_nUpdatedFiles, updatePackage.m_nTotalFiles);
	m_statusCallback->updateStatus(nCurrentIndex, finishText );

	ui.SaveToFile(ui.m_FileName);
	m_nSuccessPackageUpdates++;
	return true;
}


CUpdatePackage::CUpdatePackage()
{
	m_statusCallback = 0;
	randomize();
	m_nUpdatedFiles = 0;
	m_nTotalFiles = 0;
	m_CoreUpdate = false;
}

bool CUpdatePackage::LoadUpdateFromFile(const CString& filename)
{
	if(!FileExists(filename)) return false;
	if(!m_xml.LoadFromFile(WCstringToUtf8(filename)))
	{
		WriteLog(logError,_T("Update Engine"),CString(_T("Failed to load update file \'"))+myExtractFileName(filename));
		return false;
	}

	TCHAR buffer[256];
	ExtractFilePath(filename, buffer);
	m_PackageFolder = buffer;

	ZSimpleXmlNode root = m_xml.getRoot("UpdatePackage", false);
	if(root.IsNull()) return false;

		
		CString packageName, UpdateUrl;
		packageName = Utf8ToWCstring(root.Attribute("Name"));
		m_TimeStamp =  root.AttributeInt("TimeStamp");
		
		int core=root.AttributeInt("CoreUpdate");
		
		m_CoreUpdate = (core != 0);
		
		ZSimpleXmlNode entry = root["Entries"];
		std::vector<ZSimpleXmlNode> entries;
		entry.GetChilds("Entry", entries);

		for(size_t i=0; i< entries.size(); i++)
		{
				CUpdateItem ui;
				ui.name = Utf8ToWCstring(entries[i].Attribute("Name"));
				ui.hash = Utf8ToWCstring(entries[i].Attribute("Hash"));
				ui.saveTo = Utf8ToWCstring(entries[i].Attribute("SaveTo"));
				ui.action = Utf8ToWCstring(entries[i].Attribute("Action"));
				
				if(ui.name.IsEmpty()  || (ui.hash.IsEmpty() &&  ui.action!=_T("delete") )|| ui.saveTo.IsEmpty())
					continue;
				m_entries.push_back(ui);
			
		}
	
	return true;

}

bool CUpdatePackage::doUpdate()
{
	for(size_t i=0; i< m_entries.size(); i++)
	{
		CUpdateItem &ue = m_entries[i];
		CString copyFrom, copyTo;
		copyFrom = m_PackageFolder + ue.name;
		copyTo = ue.saveTo;
		if( (ue.hash != IU_md5_file(copyFrom) || ue.hash.IsEmpty()) && ue.action != _T("delete"))
		{
			setStatusText( CString(TR("Не совпал MD5 хэш файла "))+myExtractFileName( copyTo));
			return false;
		}
	}

	for(size_t i=0; i< m_entries.size(); i++)
	{
		CUpdateItem &ue = m_entries[i];
	
		CString copyFrom, copyTo;
		copyFrom = m_PackageFolder + ue.name;
		copyTo = ue.saveTo;
		CString appFolder = GetAppFolder();
		if(appFolder.Right(1) == _T("\\"))
			appFolder.Delete(appFolder.GetLength()-1);

		copyTo.Replace(_T("%datapath%"), IU_GetDataFolder());
		copyTo.Replace(_T("%apppath%"), appFolder);
		CString renameTo = copyTo + _T(".")+IntToStr(random(10000))+ _T(".old");
		TCHAR buffer[MAX_PATH];
		ExtractFilePath(copyTo, buffer);
		
		m_nTotalFiles ++;

		if(ue.action == _T("delete"))
		{
			if(MoveFile(copyTo,renameTo))
				m_nUpdatedFiles++;
				DeleteFile(renameTo);			
		}
		else
		{
			CreateFolder(buffer);
			if(m_CoreUpdate)
			{
				
				MoveFile(copyTo,renameTo); 
			}
			setStatusText( _T("Copying file '") + copyFrom + _T("' to location '") + copyTo);
			

			
			if(!CopyFile(copyFrom,copyTo,FALSE))
			{
				WriteLog(logWarning,_T("Update Engine"),CString(_T("Не могу записать файл "))+myExtractFileName(copyTo));
				
			}
			else 
			{
				m_nUpdatedFiles++;
				if(m_CoreUpdate) DeleteFile(renameTo);
			}
		}
	}
	DeleteDir2(m_PackageFolder);
	return true;
}

void CUpdatePackage::setStatusText(const CString& text)
{
	if(m_statusCallback)
		m_statusCallback->updateStatus(0, text);
}

CUpdateManager::CUpdateManager()
{
	m_statusCallback= 0;
	m_nCoreUpdates = 0;
	
	m_nSuccessPackageUpdates = 0;
	m_stop = false;
	nm.setProgressCallback(progressCallback, this);
}

CString CUpdateManager::generateReport()
{
	CString text;
	
	for(size_t i=0; i<m_updateList.size(); i++)
	{
		time_t t = m_updateList[i].m_TimeStamp;
		tm * timeinfo = localtime ( &t );
		CString date;
		date.Format(_T("[%02d.%02d.%04d]"),(int)timeinfo->tm_mday,(int) timeinfo->tm_mon+1, (int)1900+timeinfo->tm_year);
		text += _T(" * ")+m_updateList[i].m_DisplayName+_T("  ")+date+_T("\r\n\r\n");
		text += m_updateList[i].m_ReadableText;
		text += _T("\r\n");		
	}
	return text;
}
void CUpdatePackage::setUpdateStatusCallback(CUpdateStatusCallback * callback)
{
	m_statusCallback = callback;
}

void CUpdateManager::setUpdateStatusCallback(CUpdateStatusCallback * callback)
{
	m_statusCallback = callback;
}
void CUpdateManager::updateStatus(int packageIndex, const CString status)
{
	if(m_statusCallback)
		m_statusCallback->updateStatus(nCurrentIndex, status);
}

bool CUpdateManager::AreUpdatesAvailable()
{
	return (m_updateList.size() != 0);
}

int CUpdateManager::progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	CUpdateManager * um = reinterpret_cast<CUpdateManager*>( clientp);
	CString text;
	CString buf1, buf2;
	buf1 = Utf8ToWCstring(IuCoreUtils::fileSizeToString(zint64(dlnow)));
	buf2 = Utf8ToWCstring(IuCoreUtils::fileSizeToString(zint64(dltotal)));
	int percent = 0;
	if(dltotal != 0 )
	percent = int((dlnow/ dltotal) * 100);
	if(percent > 100) percent = 0;
	text.Format(TR("Скачано %s из %s (%d %%)"),(LPCTSTR)buf1, (LPCTSTR)buf2,percent);
	um->updateStatus(0, text);
	if(um->m_stop) return 1;
	return 0;
}

void CUpdateManager::Clear()
{
	m_updateList.clear();
	m_nCoreUpdates = 0;
	m_nSuccessPackageUpdates = 0;
	m_stop = false;
}