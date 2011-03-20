#pragma once

#include "Common/MyXML.h"
#include "Core/Network/NetworkManager.h"
#include "Core/Utils/SimpleXml.h"

class CUpdateItem
{
	public:
		CString name;
		CString hash;
		CString saveTo;
		CString action;
};

class CUpdateInfo
{
public:
	CUpdateInfo();
	bool LoadUpdateFromFile(const CString& filename);
	bool LoadUpdateFromBuffer(const CString& buffer);
	bool DoUpdate(const CUpdateInfo &newPackage);
	bool SaveToFile(const CString& filename);
	bool Parse(ZSimpleXml &xml);
	
	CString m_ReadableText;
	CString m_PackageName, m_DownloadUrl, m_UpdateUrl,m_Hash;
	bool CheckUpdates();
	CString m_FileName;
	const CString getHash();
	bool m_CoreUpdate;
	int m_TimeStamp;
	CString m_DisplayName;
	CString m_Buffer;
	bool CanUpdate(const CUpdateInfo& newInfo);
bool	operator<(const CUpdateInfo& p);
};

class CUpdateStatusCallback
{
	public:
		virtual void updateStatus(int packageIndex, const CString status)=0;
};

class CUpdatePackage
{
public:
	ZSimpleXml m_xml;
	CString m_PackageName;
	CString m_PackageFolder;
	int m_TimeStamp;
	bool m_CoreUpdate;
	CUpdatePackage();
	std::vector< CUpdateItem> m_entries;
	bool LoadUpdateFromFile(const CString& filename);
	bool doUpdate();
	int m_nUpdatedFiles;
	int m_nTotalFiles;
	CUpdateStatusCallback* m_statusCallback;
	void setUpdateStatusCallback(CUpdateStatusCallback * callback);
	void setStatusText(const CString& text);
};


class CUpdateManager: public CUpdateStatusCallback
{
public:
	int nCurrentIndex;
	CUpdateManager();
	bool CheckUpdates();
	CUpdateStatusCallback *m_statusCallback;
	bool DoUpdates();
	const CString ErrorString();
	NetworkManager nm;
	static int progressCallback(void *clientp,
                                      double dltotal,
                                      double dlnow,
                                      double ultotal,
                                      double ulnow);
	std::vector<CUpdateInfo> m_updateList;
	CString generateReport();
	int m_nSuccessPackageUpdates;
	bool m_stop;
	int m_nCoreUpdates;
	void Clear();
	bool AreUpdatesAvailable();
	bool AreCoreUpdates();
	void updateStatus(int packageIndex, const CString status);
	bool internal_load_update(CString name);
	bool internal_do_update(CUpdateInfo& ui);
	void setUpdateStatusCallback(CUpdateStatusCallback * callback);
private:
	CString m_ErrorStr;
};

