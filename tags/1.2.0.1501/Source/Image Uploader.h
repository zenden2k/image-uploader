

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Thu Dec 25 15:02:52 2008
 */
/* Compiler settings for .\aviinfo.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __Image_Uploader_h__
#define __Image_Uploader_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef ___DAviInfoCtrl_FWD_DEFINED__
#define ___DAviInfoCtrl_FWD_DEFINED__
typedef interface _DAviInfoCtrl _DAviInfoCtrl;
#endif 	/* ___DAviInfoCtrl_FWD_DEFINED__ */


#ifndef ___DAviInfoCtrlEvents_FWD_DEFINED__
#define ___DAviInfoCtrlEvents_FWD_DEFINED__
typedef interface _DAviInfoCtrlEvents _DAviInfoCtrlEvents;
#endif 	/* ___DAviInfoCtrlEvents_FWD_DEFINED__ */


#ifndef __AviInfoCtrl_FWD_DEFINED__
#define __AviInfoCtrl_FWD_DEFINED__

#ifdef __cplusplus
typedef class AviInfoCtrl AviInfoCtrl;
#else
typedef struct AviInfoCtrl AviInfoCtrl;
#endif /* __cplusplus */

#endif 	/* __AviInfoCtrl_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __AVIINFOCTRLLib_LIBRARY_DEFINED__
#define __AVIINFOCTRLLib_LIBRARY_DEFINED__

/* library AVIINFOCTRLLib */
/* [custom][custom][custom][helpcontext][helpfile][helpstring][version][uuid] */ 




EXTERN_C const IID LIBID_AVIINFOCTRLLib;

#ifndef ___DAviInfoCtrl_DISPINTERFACE_DEFINED__
#define ___DAviInfoCtrl_DISPINTERFACE_DEFINED__

/* dispinterface _DAviInfoCtrl */
/* [hidden][helpstring][uuid] */ 


EXTERN_C const IID DIID__DAviInfoCtrl;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("059660A2-D7F2-44B5-B14F-279996C1EE56")
    _DAviInfoCtrl : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DAviInfoCtrlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DAviInfoCtrl * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DAviInfoCtrl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DAviInfoCtrl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DAviInfoCtrl * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DAviInfoCtrl * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DAviInfoCtrl * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DAviInfoCtrl * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DAviInfoCtrlVtbl;

    interface _DAviInfoCtrl
    {
        CONST_VTBL struct _DAviInfoCtrlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DAviInfoCtrl_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _DAviInfoCtrl_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _DAviInfoCtrl_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _DAviInfoCtrl_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _DAviInfoCtrl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _DAviInfoCtrl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _DAviInfoCtrl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DAviInfoCtrl_DISPINTERFACE_DEFINED__ */


#ifndef ___DAviInfoCtrlEvents_DISPINTERFACE_DEFINED__
#define ___DAviInfoCtrlEvents_DISPINTERFACE_DEFINED__

/* dispinterface _DAviInfoCtrlEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DAviInfoCtrlEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("5AD53123-E109-4EA3-BC35-A7688FFD8E2F")
    _DAviInfoCtrlEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DAviInfoCtrlEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DAviInfoCtrlEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DAviInfoCtrlEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DAviInfoCtrlEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DAviInfoCtrlEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DAviInfoCtrlEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DAviInfoCtrlEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DAviInfoCtrlEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DAviInfoCtrlEventsVtbl;

    interface _DAviInfoCtrlEvents
    {
        CONST_VTBL struct _DAviInfoCtrlEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DAviInfoCtrlEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _DAviInfoCtrlEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _DAviInfoCtrlEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _DAviInfoCtrlEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _DAviInfoCtrlEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _DAviInfoCtrlEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _DAviInfoCtrlEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DAviInfoCtrlEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_AviInfoCtrl;

#ifdef __cplusplus

class DECLSPEC_UUID("BE471E9C-559C-4E71-B503-BAE8E1E34353")
AviInfoCtrl;
#endif
#endif /* __AVIINFOCTRLLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


