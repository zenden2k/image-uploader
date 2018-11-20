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
    HRESULT __stdcall GetTypeInfoCount(UINT* pctinfo) override;
    HRESULT __stdcall GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    HRESULT __stdcall GetIDsOfNames(const IID& riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override;
    HRESULT __stdcall Invoke(DISPID dispIdMember, const IID& riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override;
    HRESULT __stdcall get_accParent(IDispatch** ppdispParent) override;
    HRESULT __stdcall get_accChildCount(long* pcountChildren) override;
    HRESULT __stdcall get_accChild(VARIANT varChild, IDispatch** ppdispChild) override;
    HRESULT __stdcall get_accName(VARIANT varChild, BSTR* pszName) override;
    HRESULT __stdcall get_accValue(VARIANT varChild, BSTR* pszValue) override;
    HRESULT __stdcall get_accDescription(VARIANT varChild, BSTR* pszDescription) override;
    HRESULT __stdcall get_accRole(VARIANT varChild, VARIANT* pvarRole) override;
    HRESULT __stdcall get_accState(VARIANT varChild, VARIANT* pvarState) override;
    HRESULT __stdcall get_accHelp(VARIANT varChild, BSTR* pszHelp) override;
    HRESULT __stdcall get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic) override;
    HRESULT __stdcall get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut) override;
    HRESULT __stdcall get_accFocus(VARIANT* pvarChild) override;
    HRESULT __stdcall get_accSelection(VARIANT* pvarChildren) override;
    HRESULT __stdcall get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction) override;
    HRESULT __stdcall accSelect(long flagsSelect, VARIANT varChild) override;
    HRESULT __stdcall accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild) override;
    HRESULT __stdcall accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt) override;
    HRESULT __stdcall accHitTest(long xLeft, long yTop, VARIANT* pvarChild) override;
    HRESULT __stdcall accDoDefaultAction(VARIANT varChild) override;
    HRESULT __stdcall put_accName(VARIANT varChild, BSTR szName) override;
    HRESULT __stdcall put_accValue(VARIANT varChild, BSTR szValue) override;
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
