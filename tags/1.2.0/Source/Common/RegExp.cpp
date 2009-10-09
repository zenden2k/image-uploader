#include "stdafx.h"
#include "regexp.h"
//---------------------------------------------------------------------------
const GUID LIBID_VBScript_RegExp_55 = {0x3F4DACA7, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID IID_IRegExp              = {0x3F4DACA0, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID IID_IMatch               = {0x3F4DACA1, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID IID_IMatchCollection     = {0x3F4DACA2, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID IID_IRegExp2             = {0x3F4DACB0, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID IID_IMatch2              = {0x3F4DACB1, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID IID_IMatchCollection2    = {0x3F4DACB2, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID IID_ISubMatches          = {0x3F4DACB3, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID CLSID_RegExp             = {0x3F4DACA4, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID CLSID_Match              = {0x3F4DACA5, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID CLSID_MatchCollection    = {0x3F4DACA6, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
const GUID CLSID_SubMatches         = {0x3F4DACC0, 0x160D, 0x11D2, {0xA8, 0xE9, 0x00, 0x10, 0x4B, 0x36, 0x5C, 0x9F}};
//---------------------------------------------------------------------------

RegExp::RegExp (void)
{
    m_regexp  = 0;
    m_matches = 0;

    if (SUCCEEDED(CoCreateInstance(CLSID_RegExp, 0, CLSCTX_ALL, IID_IRegExp2, (void**)&m_regexp)))
    {
        m_regexp->set_IgnoreCase(VARIANT_TRUE);
        m_regexp->set_Multiline(VARIANT_TRUE);
        m_regexp->set_Global(VARIANT_TRUE);
    }
}
//---------------------------------------------------------------------------

RegExp::~RegExp (void)
{
    if (m_matches)
    {
        m_matches->Release();
        m_matches = 0;
    }
    
    if (m_regexp)
    {
        m_regexp->Release();
        m_regexp = 0;
    }
}
//---------------------------------------------------------------------------

bool RegExp::SetPattern (const BSTRString &pattern)
{
    if (!m_regexp)
        return false;

    if (m_matches)
    {
        m_matches->Release();
        m_matches = 0;
    }

    return SUCCEEDED(m_regexp->set_Pattern((BSTR)pattern/*.Data()*/));
}
//---------------------------------------------------------------------------

bool RegExp::Execute (const BSTRString &source)
{
    if (!m_regexp)
        return false;

    if (m_matches)
    {
        m_matches->Release();
        m_matches = 0;
    }

    IDispatch* disp = 0;
    if (FAILED(m_regexp->Execute(source/*.Data()*/, &disp)))
        return false;

    if (FAILED(disp->QueryInterface(IID_IMatchCollection2, (void**)&m_matches)))
    {
        disp->Release();
        return false;
    }

    disp->Release();

    return MatchCount() != 0;
}
//---------------------------------------------------------------------------

bool RegExp::Execute (const BSTRString &source, const BSTRString &pattern)
{
    return SetPattern(pattern) && Execute(source);
}
//---------------------------------------------------------------------------

bool RegExp::Test (const BSTRString &source)
{
    if (!m_regexp)
        return false;

    if (m_matches)
    {
        m_matches->Release();
        m_matches = 0;
    }

    VARIANT_BOOL vbResult = VARIANT_FALSE;
    
    if (FAILED(m_regexp->Test(source/*.Data()*/, &vbResult)))
        return false;

    return vbResult == VARIANT_TRUE;
}
//---------------------------------------------------------------------------

bool RegExp::Test (const BSTRString &source, const BSTRString &pattern)
{
    return SetPattern(pattern) && Test(source);
}
//---------------------------------------------------------------------------

DWORD RegExp::MatchCount (void)
{
    if (!m_matches)
        return 0;
    
    long lCount = 0;

    if (FAILED(m_matches->get_Count(&lCount)))
        return 0;

    return (DWORD)lCount;
}
//---------------------------------------------------------------------------

BSTRString RegExp::GetMatch (DWORD dwMatchIndex)
{
    DWORD dwCount = MatchCount();
    
    if (dwMatchIndex >= dwCount)
        return TEXT("");

    IMatch2*   match = 0;
    IDispatch* disp  = 0;
    if (FAILED(m_matches->get_Item(dwMatchIndex, &disp)))
        return TEXT("");

    if (FAILED(disp->QueryInterface(IID_IMatch2, (void**)&match)))
    {
        disp->Release();
        return TEXT("");
    }

    disp->Release();

    BSTR bstrMatch = 0;
    if (FAILED(match->get_Value(&bstrMatch)))
    {
        match->Release();
        return TEXT("");
    }

    BSTRString Match;
    Match.Attach(bstrMatch);
    
    match->Release();

    return Match;
}
//---------------------------------------------------------------------------

DWORD RegExp::SubMatchCount (DWORD dwMatchIndex)
{
    DWORD dwCount = MatchCount();
    
    if (dwMatchIndex >= dwCount)
        return 0;

    IMatch2*   match = 0;
    IDispatch* disp  = 0;
    if (FAILED(m_matches->get_Item(dwMatchIndex, &disp)))
        return 0;

    if (FAILED(disp->QueryInterface(IID_IMatch2, (void**)&match)))
    {
        disp->Release();
        return 0;
    }

    disp->Release();
    disp = 0;

    ISubMatches* sub_matches = 0;
    if (FAILED(match->get_SubMatches(&disp)))
    {
        match->Release();
        return 0;
    }

    if (FAILED(disp->QueryInterface(IID_ISubMatches, (void**)&sub_matches)))
    {
        disp->Release();
        match->Release();
        return 0;
    }

    disp->Release();

    long lCount = 0;
    if (FAILED(sub_matches->get_Count(&lCount)))
    {
        sub_matches->Release();
        match->Release();
        return 0;
    }

    sub_matches->Release();
    match->Release();

    return (DWORD)lCount;
}
//---------------------------------------------------------------------------

BSTRString RegExp::GetSubMatch (DWORD dwMatchIndex, DWORD dwSubMatchIndex)
{
    DWORD dwCount = MatchCount();
    
    if (dwMatchIndex >= dwCount)
        return TEXT("");

    IMatch2*   match = 0;
    IDispatch* disp  = 0;
    if (FAILED(m_matches->get_Item(dwMatchIndex, &disp)))
        return TEXT("");

    if (FAILED(disp->QueryInterface(IID_IMatch2, (void**)&match)))
    {
        disp->Release();
        return TEXT("");
    }

    disp->Release();
    disp = 0;

    ISubMatches* sub_matches = 0;
    if (FAILED(match->get_SubMatches(&disp)))
    {
        match->Release();
        return TEXT("");
    }

    if (FAILED(disp->QueryInterface(IID_ISubMatches, (void**)&sub_matches)))
    {
        disp->Release();
        match->Release();
        return TEXT("");
    }

    disp->Release();

    long lCount = 0;
    if (FAILED(sub_matches->get_Count(&lCount)) || dwSubMatchIndex >= (DWORD)lCount)
    {
        sub_matches->Release();
        match->Release();
        return TEXT("");
    }

    VARIANT v;
    VariantInit(&v);
    if (FAILED(sub_matches->get_Item(dwSubMatchIndex, &v)))
    {
        sub_matches->Release();
        match->Release();
        return TEXT("");
    }

    if (FAILED(VariantChangeType(&v, &v, 0, VT_BSTR)))
    {
        sub_matches->Release();
        match->Release();
        VariantClear(&v);
        return TEXT("");
    }

    BSTRString SubMatch;
    SubMatch.Attach(v.bstrVal);

    sub_matches->Release();
    match->Release();

    return SubMatch;
}
//---------------------------------------------------------------------------

 BSTRString RegExp::Replace (const BSTRString &source, const BSTRString &replace)
{
    if (!m_regexp)
        return TEXT("");

    if (m_matches)
    {
        m_matches->Release();
        m_matches = 0;
    }

    BSTR bstrResult = 0;

    VARIANT varReplace;
    VariantInit(&varReplace);

    varReplace.vt      = VT_BSTR;
    varReplace.bstrVal = replace/*.Data()*/;

    if (FAILED(m_regexp->Replace(source/*.Data()*/, varReplace, &bstrResult)))
        return TEXT("");

    BSTRString result;
    result.Attach(bstrResult);

    return result;
}
//---------------------------------------------------------------------------

BSTRString RegExp::Replace (const BSTRString &source, const BSTRString &pattern, const BSTRString &replace)
{
    if (!SetPattern(pattern))
        return TEXT("");

    return Replace(source, replace);
}
//---------------------------------------------------------------------------