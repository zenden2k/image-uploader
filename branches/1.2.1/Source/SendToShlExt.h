// SendToShlExt.h : Declaration of the CSendToShlExt

#pragma once
#include "resource.h"       // main symbols
#include "IUCom.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// CSendToShlExt

class ATL_NO_VTABLE CSendToShlExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSendToShlExt, &CLSID_SendToShlExt>,
	public IDispatchImpl<ISendToShlExt, &IID_ISendToShlExt, &LIBID_IUComLib, /*wMajor =*/ 1, /*wMinor =*/ 0>, public IDropTarget
	
{
	public:
		CSendToShlExt()
		{
		}

	DECLARE_REGISTRY_RESOURCEID(IDR_REGISTRY1)

	BEGIN_COM_MAP(CSendToShlExt)
		COM_INTERFACE_ENTRY(ISendToShlExt)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IDropTarget)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	public:
		protected:
		TCHAR buffer[10*1024];

	public:
		// IPersistFile
		STDMETHOD(GetClassID)(LPCLSID)      { return E_NOTIMPL; }
		STDMETHOD(IsDirty)()                { return E_NOTIMPL; }
		STDMETHOD(Load)(LPCOLESTR, DWORD)   { return S_OK;      }
		STDMETHOD(Save)(LPCOLESTR, BOOL)    { return E_NOTIMPL; }
		STDMETHOD(SaveCompleted)(LPCOLESTR) { return E_NOTIMPL; }
		STDMETHOD(GetCurFile)(LPOLESTR*)    { return E_NOTIMPL; }

		// IDropTarget
		STDMETHOD(DragEnter)(IDataObject* pDataObj, DWORD grfKeyState,
							 POINTL pt, DWORD* pdwEffect);

		STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
			{ return E_NOTIMPL; }

		STDMETHOD(DragLeave)()
			{ return S_OK; }

		STDMETHOD(Drop)(IDataObject* pDataObj, DWORD grfKeyState,
						POINTL pt, DWORD* pdwEffect);

};

OBJECT_ENTRY_AUTO(__uuidof(SendToShlExt), CSendToShlExt)
