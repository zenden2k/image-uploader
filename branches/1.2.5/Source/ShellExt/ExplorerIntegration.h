

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Oct 14 15:23:16 2009
 */
/* Compiler settings for .\ExplorerIntegration.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ExplorerIntegration_h__
#define __ExplorerIntegration_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IIShellContextMenu_FWD_DEFINED__
#define __IIShellContextMenu_FWD_DEFINED__
typedef interface IIShellContextMenu IIShellContextMenu;
#endif 	/* __IIShellContextMenu_FWD_DEFINED__ */


#ifndef __IShellContextMenu_FWD_DEFINED__
#define __IShellContextMenu_FWD_DEFINED__

#ifdef __cplusplus
typedef class IShellContextMenu IShellContextMenu;
#else
typedef struct IShellContextMenu IShellContextMenu;
#endif /* __cplusplus */

#endif 	/* __IShellContextMenu_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IIShellContextMenu_INTERFACE_DEFINED__
#define __IIShellContextMenu_INTERFACE_DEFINED__

/* interface IIShellContextMenu */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IIShellContextMenu;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2AAE4044-B77F-4BF2-95EB-F6AD0A3332C8")
    IIShellContextMenu : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IIShellContextMenuVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IIShellContextMenu * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IIShellContextMenu * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IIShellContextMenu * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IIShellContextMenu * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IIShellContextMenu * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IIShellContextMenu * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IIShellContextMenu * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IIShellContextMenuVtbl;

    interface IIShellContextMenu
    {
        CONST_VTBL struct IIShellContextMenuVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IIShellContextMenu_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IIShellContextMenu_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IIShellContextMenu_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IIShellContextMenu_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IIShellContextMenu_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IIShellContextMenu_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IIShellContextMenu_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IIShellContextMenu_INTERFACE_DEFINED__ */



#ifndef __ExplorerIntegrationLib_LIBRARY_DEFINED__
#define __ExplorerIntegrationLib_LIBRARY_DEFINED__

/* library ExplorerIntegrationLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ExplorerIntegrationLib;

EXTERN_C const CLSID CLSID_IShellContextMenu;

#ifdef __cplusplus

class DECLSPEC_UUID("7CA585C4-DF37-4712-9FC7-D64BFEF801F9")
IShellContextMenu;
#endif
#endif /* __ExplorerIntegrationLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


