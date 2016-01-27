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

#include "UpdatePackage.h"

#include <time.h>
#include "atlheaders.h"
#include "3rdpart/unzipper.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Core/Utils/StringUtils.h"
#include "Func/Common.h"
#include "Core/Utils/CryptoUtils.h"
#include "WinUtils.h"
#include <iostream>
#include "IuCommonFunctions.h"
#include "Core/Utils/SystemUtils.h"
#include "Core/CommonDefs.h"

#ifdef IU_CLI
#undef TR
#define TR(str) _T(str)

#endif
#include "Core/ServiceLocator.h"
#include "Core/CoreFunctions.h""

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
    SimpleXml xml;
    if(!xml.LoadFromFile(W2U(filename)))    
    {
        ServiceLocator::instance()->logger()->write(logError, _T("Update Engine"), CString(_T("Failed to load update file ")) + filename + _T("\r\n"));
    }
    m_FileName = filename;
    Parse(xml);
    return true;
}

bool CUpdateInfo::SaveToFile(const CString& filename)
{
    FILE *f = _wfopen(filename, _T("wb"));
    if(!f) return false;
    
    std::string outbuf = W2U(m_Buffer);
    fwrite(outbuf.c_str(), 1, outbuf.size(), f);
    fclose(f);
    return false;
}

bool CUpdateInfo::LoadUpdateFromBuffer(const CString& buffer)
{
    SimpleXml m_xml;
    if(!m_xml.LoadFromString(W2U(buffer)))
    {
        ServiceLocator::instance()->logger()->write(logError, _T("Update Engine"), CString(_T("Failed to load update file \r\n")) + _T("\r\nServer answer:\r\n") + buffer);
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

bool CUpdateInfo::Parse( SimpleXml &xml)
{
    SimpleXmlNode root = xml.getRoot("UpdateInfo", false);
    if(root.IsNull()) return false;
    
    CString packageName, UpdateUrl;
    m_PackageName = IuCoreUtils::Utf8ToWstring(root.Attribute("Name")).c_str();
    m_UpdateUrl = IuCoreUtils::Utf8ToWstring(root.Attribute("UpdateUrl")).c_str();
    m_DownloadUrl = IuCoreUtils::Utf8ToWstring(root.Attribute("DownloadUrl")).c_str();
    m_Hash = IuCoreUtils::Utf8ToWstring(root.Attribute("Hash")).c_str();
    m_TimeStamp = root.AttributeInt("TimeStamp");
    m_DisplayName = IuCoreUtils::Utf8ToWstring(root.Attribute("DisplayName")).c_str();
        
    if(m_PackageName.IsEmpty() || m_UpdateUrl.IsEmpty() || m_DownloadUrl.IsEmpty() || m_Hash.IsEmpty()  || !m_TimeStamp)
        return false;
    int core = 0;
    core = root.AttributeInt("CoreUpdate");
    m_CoreUpdate = core!=0;
        
        //if(m_xml.FindElem(_T("Info")))
    //    {
    m_ReadableText = IuCoreUtils::Utf8ToWstring(root["Info"].Text()).c_str();
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

bool CUpdateInfo::isCoreUpdate() const
{
    return m_CoreUpdate;
}

CString CUpdateInfo::displayName() const
{
    return m_DisplayName;
}

CString CUpdateInfo::downloadUrl() const
{
    return m_DownloadUrl;
}

CString CUpdateInfo::fileName() const
{
    return m_FileName;
}

CString CUpdateInfo::packageName() const
{
    return m_PackageName;
}

CString CUpdateInfo::readableText() const
{
    return m_ReadableText;
}

CString CUpdateInfo::updateUrl() const
{
    return m_UpdateUrl;
}

int CUpdateInfo::timeStamp() const
{
    return m_TimeStamp;
}

void CUpdateInfo::setFileName(const CString & name)
{
    m_FileName = name;
}

bool CUpdateManager::CheckUpdates()
{
    m_ErrorStr.Empty();
    Clear();
    std::vector<CString> fileList;
    WinUtils::GetFolderFileList(fileList, IuCommonFunctions::GetDataFolder() + _T("Update"), _T("*.xml"));

    bool Result = true;

    if (fileList.size() == 0) {
        m_ErrorStr = "No update files found in folder '" + IuCommonFunctions::GetDataFolder() + _T("Update\\'");
        return false;
    }

    for (size_t i = 0; i < fileList.size(); i++) {
        CString fileName = IuCommonFunctions::GetDataFolder() + _T("Update\\") + fileList[i];
        if (!internal_load_update(fileName))
            Result = false;
    }

    return Result;
}

const CString CUpdateManager::ErrorString()
{
    return m_ErrorStr;
}

bool CUpdateManager::DoUpdates()
{
    for (size_t i = 0; i < m_updateList.size(); i++) {
        nCurrentIndex = i;
        if (m_stop) return 0;
        internal_do_update(m_updateList[i]);
    }
    return true;
}

bool CUpdateManager::internal_load_update(CString name)
{
    CUpdateInfo localPackage;

    if (!localPackage.LoadUpdateFromFile(name)) {
        ServiceLocator::instance()->logger()->write(logError, _T("Update Engine"), CString(TR("Could not download the update file \\'")) + name);
        return false;
    }

    CUpdateInfo remotePackage;
    NetworkClient nm;
    nm.setTreatErrorsAsWarnings(true);
    nm.enableResponseCodeChecking(false);
    nm.setProgressCallback(NetworkClient::ProgressCallback(this,&CUpdateManager::progressCallback));
    CoreFunctions::ConfigureProxy(&nm);

    CString url = localPackage.updateUrl();
    url.Replace(_T("%appver%"), IuCommonFunctions::GetVersion());
    url.Replace(_T("%name%"), localPackage.packageName());
    url.Replace(_T("OS_NAME"), U2W(nm.urlEncode(IuCoreUtils::GetOsName())));
    url.Replace(_T("OS_VER"), U2W(nm.urlEncode(IuCoreUtils::GetOsVersion())));
    url.Replace(_T("CPU_FEATURES"), U2W(nm.urlEncode(IuCoreUtils::GetCpuFeatures())));
    try {
        nm.doGet(W2U(url));
    } catch ( NetworkClient::AbortedException&) {
        return false;
    }
   
    if(nm.responseCode() != 200)
    {
        ServiceLocator::instance()->logger()->write(logWarning, _T("Update Engine"), _T("Невозможно загрузить информацию о пакете обновления ") + localPackage.packageName() + CString(_T("\r\nHTTP response code: ")) + IuCoreUtils::Utf8ToWstring(IuCoreUtils::int64_tToString(nm.responseCode())).c_str() + _T("\r\n") + IuCoreUtils::Utf8ToWstring(nm.errorString()).c_str(), CString("URL=") + url);
        return false;
    }

    std::wstring res = IuCoreUtils::Utf8ToWstring( nm.responseBody());
    if(!remotePackage.LoadUpdateFromBuffer(res.c_str()))
    {
        return false;
    }

    if(!localPackage.CanUpdate(remotePackage )) return true;
    remotePackage.setFileName(localPackage.fileName());

    m_updateList.push_back(remotePackage);
    if(remotePackage.isCoreUpdate()) m_nCoreUpdates++;    
    return true;
}

bool CUpdateManager::AreCoreUpdates()
{
    return m_nCoreUpdates!=0;
}

bool CUpdateManager::internal_do_update(CUpdateInfo& ui)
{
    CString filename = IuCommonFunctions::IUTempFolder + ui.packageName() +_T(".zip");
    std::string filenamea = W2U(filename);
    CoreFunctions::ConfigureProxy(&nm_);
    nm_.setOutputFile( filenamea);
    m_statusCallback->updateStatus(nCurrentIndex, TR("Downloading file ")+ ui.downloadUrl());

    try {
        nm_.doGet(W2U(ui.downloadUrl()));
    } catch (NetworkClient::AbortedException&) {
        return false;
    }
    
    if(nm_.responseCode() != 200)
    {
        ServiceLocator::instance()->logger()->write(logError, _T("Update Engine"), TR("Error while updating component ") + ui.packageName() + CString(_T("\r\nHTTP response code: ")) + IuCoreUtils::Utf8ToWstring(IuCoreUtils::int64_tToString(nm_.responseCode())).c_str() + _T("\r\n") + IuCoreUtils::Utf8ToWstring(nm_.errorString()).c_str(), CString("URL=") + ui.downloadUrl());
        return 0;
    }

    CString hash = ui.getHash();
    hash.MakeLower();
    if( hash != IuCoreUtils::Utf8ToWstring(IuCoreUtils::CryptoUtils::CalcMD5HashFromFile(IuCoreUtils::WstringToUtf8((LPCTSTR)filename))).c_str() || ui.getHash().IsEmpty())
    {
        updateStatus(0, CString(TR("MD5 check of the update package failed "))+IuCoreUtils::ExtractFileName(IuCoreUtils::WstringToUtf8((LPCTSTR)filename)).c_str());
        return 0;
    }

    CUnzipper unzipper(filename);
    CString unzipFolder = IuCommonFunctions::IUTempFolder + ui.packageName();
    if(!unzipper.UnzipTo(unzipFolder))
    {
        updateStatus(0, TR("Unable to unpack archive ")+ filename);
        return 0;
    }

    CUpdatePackage updatePackage;
    updatePackage.setUpdateStatusCallback(this);
    if(!updatePackage.LoadUpdateFromFile(unzipFolder + _T("\\")+_T("package.xml")))
    {
        MessageBox(0,TR("Could not read ") + ui.packageName(),0,0);
        return false;
    }

    if(!updatePackage.doUpdate())
        return false;
    CString finishText;
    finishText.Format(TR("Update finished. Updated %d of %d files "), updatePackage.updatedFileCount(), updatePackage.totalFileCount());
    m_statusCallback->updateStatus(nCurrentIndex, finishText );

    ui.SaveToFile(ui.fileName());
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
    if(!IuCoreUtils::FileExists(W2U(filename))) return false;
    if(!m_xml.LoadFromFile(W2U(filename))) {
        ServiceLocator::instance()->logger()->write(logError, _T("Update Engine"), CString(_T("Failed to load update file \'")) + IuCoreUtils::Utf8ToWstring(IuCoreUtils::ExtractFileName(IuCoreUtils::WstringToUtf8((LPCTSTR)filename))).c_str());
        return false;
    }
    
    m_PackageFolder = IuCoreUtils::Utf8ToWstring(IuCoreUtils::ExtractFilePath(W2U(filename))).c_str();
    m_PackageFolder += "\\";
    SimpleXmlNode root = m_xml.getRoot("UpdatePackage", false);
    if(root.IsNull()) return false;

    CString packageName, UpdateUrl;
    packageName = IuCoreUtils::Utf8ToWstring(root.Attribute("Name")).c_str();
    m_TimeStamp =  root.AttributeInt("TimeStamp");
        
    int core=root.AttributeInt("CoreUpdate");
        
    m_CoreUpdate = (core != 0);
        
    SimpleXmlNode entry = root["Entries"];
    std::vector<SimpleXmlNode> entries;
    entry.GetChilds("Entry", entries);

    for(size_t i=0; i< entries.size(); i++){
        CUpdateItem ui;
        ui.name = IuCoreUtils::Utf8ToWstring(entries[i].Attribute("Name")).c_str();
        ui.hash = IuCoreUtils::Utf8ToWstring(entries[i].Attribute("Hash")).c_str();
        ui.saveTo = IuCoreUtils::Utf8ToWstring(entries[i].Attribute("SaveTo")).c_str();
        ui.action = IuCoreUtils::Utf8ToWstring(entries[i].Attribute("Action")).c_str();
        ui.flags = entries[i].Attribute("Flags");
        if(ui.name.IsEmpty()  || (ui.hash.IsEmpty() &&  ui.action!=_T("delete") )|| ui.saveTo.IsEmpty())
            continue;
        m_entries.push_back(ui);
        }    
    return true;
}

int CUpdatePackage::updatedFileCount() const
{
    return m_nUpdatedFiles;
}

int CUpdatePackage::totalFileCount() const
{
    return m_nTotalFiles;
}

bool CUpdatePackage::doUpdate()
{
    for(size_t i=0; i< m_entries.size(); i++)
    {
        CUpdateItem &ue = m_entries[i];
        CString copyFrom, copyTo;
        copyFrom = m_PackageFolder + ue.name;
        copyTo = ue.saveTo;
        if (ue.action != _T("delete") && (ue.hash != IuCoreUtils::Utf8ToWstring(IuCoreUtils::CryptoUtils::CalcMD5HashFromFile(IuCoreUtils::WstringToUtf8((LPCTSTR)copyFrom))).c_str() || ue.hash.IsEmpty()))
        {
            /*std::cout << std::endl << IuCoreUtils::WstringToUtf8((LPCTSTR)copyFrom)<<std::endl;
            std::cout <<  IuCoreUtils::CryptoUtils::CalcMD5HashFromFile(IuCoreUtils::WstringToUtf8((LPCTSTR)copyFrom))<<std::endl;
            std::cout << IuCoreUtils::WstringToUtf8((LPCTSTR)ue.hash) << std::endl;*/
            setStatusText( CString(TR("MD5 check failed for file "))+IuCoreUtils::Utf8ToWstring(IuCoreUtils::ExtractFileName(IuCoreUtils::WstringToUtf8((LPCTSTR)copyTo))).c_str());
            return false;
        }
    }

    for(size_t i=0; i< m_entries.size(); i++)
    {
        CUpdateItem &ue = m_entries[i];
    
        CString copyFrom, copyTo;
        copyFrom = m_PackageFolder + ue.name;
        copyTo = ue.saveTo;
        CString appFolder = WinUtils::GetAppFolder();
        if(appFolder.Right(1) == _T("\\"))
            appFolder.Delete(appFolder.GetLength()-1);

        copyTo.Replace(_T("%datapath%"), IuCommonFunctions::GetDataFolder());
        copyTo.Replace(_T("%apppath%"), appFolder);
        std::string dir = IuCoreUtils::ExtractFilePath(IuCoreUtils::WstringToUtf8((LPCTSTR)copyTo));
        if ( !IuCoreUtils::DirectoryExists(dir)) {
            if ( !IuCoreUtils::createDirectory(dir) ) {
                CString logMessage;
                logMessage.Format(_T("Could not create folder '%s'."), (LPCTSTR)IuCoreUtils::Utf8ToWstring(dir).c_str());
                ServiceLocator::instance()->logger()->write(logError, _T("Update Engine"), logMessage);

            }
        }
        CString renameTo = copyTo + _T(".")+IuCoreUtils::Utf8ToWstring(IuCoreUtils::int64_tToString(random(10000))).c_str()+ _T(".old");

        CString buffer = U2W(IuCoreUtils::ExtractFilePath(W2U(copyTo)));
        std::vector<std::string> tokens;
        IuStringUtils::Split(ue.flags, ",", tokens, -1);
        bool skipFile = false;
        bool isWin64Os = WinUtils::IsWindows64Bit();
        for(size_t i=0; i<tokens.size(); i++)
        {
            if(tokens[i] == "os_win64bit")
            {
                skipFile = !isWin64Os;
            }
            else if(tokens[i] == "os_win32bit")
            {
                skipFile = isWin64Os;
            }
        }
        if(skipFile) {
            continue;
        }
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
                ServiceLocator::instance()->logger()->write(logWarning, _T("Update Engine"), CString(_T("Could not write file ")) + IuCoreUtils::Utf8ToWstring(IuCoreUtils::ExtractFileName(IuCoreUtils::WstringToUtf8((LPCTSTR)copyTo))).c_str());
                
            }
            else 
            {
                m_nUpdatedFiles++;
                if(m_CoreUpdate) DeleteFile(renameTo);
            }
        }
    }
    WinUtils::DeleteDir2(m_PackageFolder);
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
    nCurrentIndex = 0;
    
    m_nSuccessPackageUpdates = 0;
    m_stop = false;
    nm_.setProgressCallback(NetworkClient::ProgressCallback(this, &CUpdateManager::progressCallback));
}

CString CUpdateManager::generateReport()
{
    CString text;
    
    for(size_t i=0; i<m_updateList.size(); i++)
    {
        time_t t = m_updateList[i].timeStamp();
        tm * timeinfo = localtime ( &t );
        CString date;
        date.Format(_T("[%02d.%02d.%04d]"), timeinfo->tm_mday, timeinfo->tm_mon+1, 1900+timeinfo->tm_year);
        text += _T(" * ")+m_updateList[i].displayName()+_T("  ")+date+_T("\r\n\r\n");
        text += m_updateList[i].readableText();
        text += _T("\r\n");        
    }
    return text;
}

CString CUpdateManager::generateUpdateMessage() {
    CString text;

    for (size_t i = 0; i<m_updateList.size(); i++) {
        time_t t = m_updateList[i].timeStamp();
        tm * timeinfo = localtime(&t);
        CString date;
        date.Format(_T("[%02d.%02d.%04d]"), timeinfo->tm_mday, timeinfo->tm_mon + 1, 1900 + timeinfo->tm_year);
        text += _T(" * ") + m_updateList[i].displayName() + _T("  ") + date + _T("\r\n\r\n");

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
void CUpdateManager::updateStatus(int packageIndex, const CString& status)
{
    if(m_statusCallback)
        m_statusCallback->updateStatus(packageIndex, status);
}

bool CUpdateManager::AreUpdatesAvailable()
{
    return (m_updateList.size() != 0);
}

int CUpdateManager::progressCallback(NetworkClient *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    CUpdateManager * um = this;
    CString text;
    CString buf1, buf2;
    buf1 = U2W(IuCoreUtils::fileSizeToString(int64_t(dlnow)));
    buf2 = U2W(IuCoreUtils::fileSizeToString(int64_t(dltotal)));
    int percent = 0;
    if(dltotal != 0 )
    percent = int((dlnow/ dltotal) * 100);
    if(percent > 100) percent = 0;
    text.Format(TR("Downloaded %s of %s (%d %%)"), static_cast<LPCTSTR>(buf1), static_cast<LPCTSTR>(buf2), percent);
    um->updateStatus(0, text);
    if(um->m_stop) return -1;
    return 0;
}

void CUpdateManager::Clear()
{
    m_updateList.clear();
    m_nCoreUpdates = 0;
    m_nSuccessPackageUpdates = 0;
    m_stop = false;
}

int CUpdateManager::successPackageUpdatesCount() const
{
    return m_nSuccessPackageUpdates;
}

void CUpdateManager::stop()
{
    m_stop = true;
}
