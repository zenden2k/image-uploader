#ifndef IU_MY_ENGINE_LIST_H
#define IU_MY_ENGINE_LIST_H

#include <atlapp.h>
#include <atlbase.h>
#include <atlmisc.h>
#include "../Core/UploadEngineList.h"
#include "../Core/Upload/DefaultUploadEngine.h"
class CMyEngineList: public CUploadEngineList
{
public:
	CMyEngineList();
	~CMyEngineList();
	CString m_ErrorStr;
	const CString ErrorStr();
	CUploadEngineData* byName(const CString &name);
	int GetUploadEngineIndex(const CString Name);
	CAbstractUploadEngine* getUploadEngine(CUploadEngineData* data);
	CAbstractUploadEngine* getUploadEngine(std::string name);
	CAbstractUploadEngine* getUploadEngine(int index);
	bool LoadFromFile(const CString filename);
private:
	CDefaultUploadEngine * m_prevUpEngine;
};
#endif