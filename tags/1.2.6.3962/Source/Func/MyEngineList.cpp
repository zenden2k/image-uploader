
#include "../MyUtils.h"
#include "../Common.h"
#include "../Settings.h"
#include "../Core/Upload/DefaultUploadEngine.h"
#include "../Core/Upload/ScriptUploadEngine.h"
#include "MyEngineList.h"
#include "../LogWindow.h"

CMyEngineList::CMyEngineList()
{
	m_prevUpEngine = 0;
} 

CMyEngineList::~CMyEngineList()
{
	delete m_prevUpEngine;
}

CUploadEngineData* CMyEngineList::byName(const CString &name)
{
return CUploadEngineList_Base::byName(WCstringToUtf8(name));
}

int CMyEngineList::GetUploadEngineIndex(const CString Name)
{
return CUploadEngineList_Base::GetUploadEngineIndex(WCstringToUtf8(Name));
}

const CString CMyEngineList::ErrorStr()
{
return m_ErrorStr;
}

CAbstractUploadEngine* CMyEngineList::getUploadEngine(CUploadEngineData* data)
{
	CAbstractUploadEngine * result = NULL;
	if(data->UsingPlugin)
	{
		result = iuPluginManager.getPlugin(data->PluginName, Settings.ServerByUtf8Name(data->Name));
	}
	else
	{
		if(m_prevUpEngine)
		{
			if(m_prevUpEngine->getUploadData()->Name == data->Name)	
				result = m_prevUpEngine;
			 else
				{
					delete m_prevUpEngine;
					m_prevUpEngine = 0;
				}
		}
	
		if(!m_prevUpEngine)
			m_prevUpEngine = new CDefaultUploadEngine();
		result = m_prevUpEngine;
	}
	if(!result)
		return 0;
	result->setUploadData(data);
	result->setServerSettings(Settings.ServerByUtf8Name(data->Name));
	result->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	return result;
}

CAbstractUploadEngine* CMyEngineList::getUploadEngine(std::string name)
{
	return getUploadEngine(CUploadEngineList_Base::byName(name));
}

CAbstractUploadEngine* CMyEngineList::getUploadEngine(int index)
{
	return getUploadEngine(CUploadEngineList_Base::byIndex(index));
}

bool CMyEngineList::LoadFromFile(const CString filename)
{
	return CUploadEngineList::LoadFromFile(WCstringToUtf8(filename));
}