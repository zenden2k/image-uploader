#ifndef IU_GUI_CONTROLS_HYPERLINKCONTROLACCESSIBLE_H
#define IU_GUI_CONTROLS_HYPERLINKCONTROLACCESSIBLE_H

#pragma once

#include <OleAcc.h>

class CHyperLinkControl;

class ATL_NO_VTABLE CHyperLinkControlAccessible
    : public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>
    , public IAccessible {
public:
    BEGIN_COM_MAP(CHyperLinkControlAccessible)
        COM_INTERFACE_ENTRY(IAccessible)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()
    /*HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObject) override;
    ULONG __stdcall AddRef() override;
    ULONG __stdcall Release() override;*/
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) override;
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    STDMETHODIMP GetIDsOfNames(const IID& riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override;
    STDMETHODIMP Invoke(DISPID dispIdMember, const IID& riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override;
    STDMETHODIMP get_accParent(IDispatch** ppdispParent) override;
    STDMETHODIMP get_accChildCount(long* pcountChildren) override;
    STDMETHODIMP get_accChild(VARIANT varChild, IDispatch** ppdispChild) override;
    STDMETHODIMP get_accName(VARIANT varChild, BSTR* pszName) override;
    STDMETHODIMP get_accValue(VARIANT varChild, BSTR* pszValue) override;
    STDMETHODIMP get_accDescription(VARIANT varChild, BSTR* pszDescription) override;
    STDMETHODIMP get_accRole(VARIANT varChild, VARIANT* pvarRole) override;
    STDMETHODIMP get_accState(VARIANT varChild, VARIANT* pvarState) override;
    STDMETHODIMP get_accHelp(VARIANT varChild, BSTR* pszHelp) override;
    STDMETHODIMP get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic) override;
    STDMETHODIMP get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut) override;
    STDMETHODIMP get_accFocus(VARIANT* pvarChild) override;
    STDMETHODIMP get_accSelection(VARIANT* pvarChildren) override;
    STDMETHODIMP get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction) override;
    STDMETHODIMP accSelect(long flagsSelect, VARIANT varChild) override;
    STDMETHODIMP accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild) override;
    STDMETHODIMP accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt) override;
    STDMETHODIMP accHitTest(long xLeft, long yTop, VARIANT* pvarChild) override;
    STDMETHODIMP accDoDefaultAction(VARIANT varChild) override;
    STDMETHODIMP put_accName(VARIANT varChild, BSTR szName) override;
    STDMETHODIMP put_accValue(VARIANT varChild, BSTR szValue) override;
public:
    static HRESULT Create(_In_ CHyperLinkControl* control, _COM_Outptr_ IAccessible** out);
    CHyperLinkControlAccessible();

    // IUnknown members.
    /*HRESULT __stdcall QueryInterface(REFIID iid, void **ppvObject);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();*/
    HRESULT CheckAlive();
    HRESULT ValidateChildId(VARIANT &varChildId);
private:
    HRESULT Initialize(_In_ CHyperLinkControl* control);
    CHyperLinkControl* control_;
    CComPtr<IAccessible> pBase_;
    BOOL alive_;
 //   ULONG refCount_;
};

#endif
