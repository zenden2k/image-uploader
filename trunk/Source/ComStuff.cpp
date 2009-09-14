#include "stdafx.h"
#include "ComStuff.h"
#include "IUCom.h"
#include "SendToShlExt.h"

class CIUComModule : public CAtlExeModuleT<CIUComModule>
{
	public :
		DECLARE_LIBID(LIBID_IUComLib)
		DECLARE_REGISTRY_APPID_RESOURCEID(IDR_REGISTRY1, "{21F419BE-54BA-4AD2-B981-D3F0BE7F999A}")
	};

	BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_SendToShlExt, CSendToShlExt)
	END_OBJECT_MAP()

bool DropTargetHandler(LPTSTR szCmdLine)
{
	TCHAR szBuffer[20];

	if(!CmdLine.IsOption(_T("-Embedding"), false))
		return false;
		
	CAtlModule* old = _pAtlModule;
	_pAtlModule = 0;
	
	CIUComModule _AtlModule;

   _AtlModule.WinMain(SW_SHOW); 
	_AtlModule.Term();
	
	IULaunchCopy();
	
	_pAtlModule = old ;
	return true;
}
