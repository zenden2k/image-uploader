#pragma once
//---------------------------------------------------------------------------
#define BSTRString CComBSTR
interface IRegExp : public IDispatch
{
    public:

        virtual HRESULT STDMETHODCALLTYPE get_Pattern    (LPBSTR pPattern)                                           = 0;
        virtual HRESULT STDMETHODCALLTYPE set_Pattern    (BSTR   pPattern)                                           = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_IgnoreCase (VARIANT_BOOL* pIgnoreCase)                                 = 0;
        virtual HRESULT STDMETHODCALLTYPE set_IgnoreCase (VARIANT_BOOL  pIgnoreCase)                                 = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Global     (VARIANT_BOOL* pGlobal)                                     = 0;
        virtual HRESULT STDMETHODCALLTYPE set_Global     (VARIANT_BOOL  pGlobal)                                     = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Execute        (BSTR sourceString, LPDISPATCH* ppMatches)                  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Test           (BSTR sourceString, VARIANT_BOOL* pMatch)                   = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Replace        (BSTR sourceString, BSTR replaceString, LPBSTR pDestString) = 0;
};
//---------------------------------------------------------------------------

interface IMatch : public IDispatch
{
    public:
        
        virtual HRESULT STDMETHODCALLTYPE get_Value      (LPBSTR pValue)      = 0;
        virtual HRESULT STDMETHODCALLTYPE get_FirstIndex (LPLONG pFirstIndex) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Length     (LPLONG pLength)     = 0;
};
//---------------------------------------------------------------------------

interface IMatchCollection : public IDispatch
{
    public:

        virtual HRESULT STDMETHODCALLTYPE get_Item     (LONG index, LPDISPATCH* ppMatch) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Count    (LPLONG pCount)                   = 0;
        virtual HRESULT STDMETHODCALLTYPE get__NewEnum (LPUNKNOWN* ppEnum)               = 0;
};
//---------------------------------------------------------------------------

interface IRegExp2 : public IDispatch
{
    public:

        virtual HRESULT STDMETHODCALLTYPE get_Pattern    (LPBSTR pPattern)                                              = 0;
        virtual HRESULT STDMETHODCALLTYPE set_Pattern    (BSTR   pPattern)                                              = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_IgnoreCase (VARIANT_BOOL* pIgnoreCase)                                    = 0;
        virtual HRESULT STDMETHODCALLTYPE set_IgnoreCase (VARIANT_BOOL  pIgnoreCase)                                    = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Global     (VARIANT_BOOL* pGlobal)                                        = 0;
        virtual HRESULT STDMETHODCALLTYPE set_Global     (VARIANT_BOOL  pGlobal)                                        = 0;

        virtual HRESULT STDMETHODCALLTYPE get_Multiline  (VARIANT_BOOL* pMultiline)                                     = 0;
        virtual HRESULT STDMETHODCALLTYPE set_Multiline  (VARIANT_BOOL  pMultiline)                                     = 0;

        virtual HRESULT STDMETHODCALLTYPE Execute        (BSTR sourceString, LPDISPATCH* ppMatches)                     = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Test           (BSTR sourceString, VARIANT_BOOL* pMatch)                      = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Replace        (BSTR sourceString, VARIANT replaceString, LPBSTR pDestString) = 0;
};
//---------------------------------------------------------------------------

interface IMatch2 : public IDispatch
{
    public:
        
        virtual HRESULT STDMETHODCALLTYPE get_Value      (LPBSTR pValue)            = 0;
        virtual HRESULT STDMETHODCALLTYPE get_FirstIndex (LPLONG pFirstIndex)       = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Length     (LPLONG pLength)           = 0;
        virtual HRESULT STDMETHODCALLTYPE get_SubMatches (LPDISPATCH* ppSubMatches) = 0;
};
//---------------------------------------------------------------------------

interface IMatchCollection2 : public IDispatch
{
    public:

        virtual HRESULT STDMETHODCALLTYPE get_Item     (LONG index, LPDISPATCH* ppMatch) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Count    (LPLONG pCount)                   = 0;
        virtual HRESULT STDMETHODCALLTYPE get__NewEnum (LPUNKNOWN* ppEnum)               = 0;
};
//---------------------------------------------------------------------------

interface ISubMatches : public IDispatch
{
    public:

        virtual HRESULT STDMETHODCALLTYPE get_Item     (LONG index, LPVARIANT pSubMatch) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Count    (LPLONG pCount)                   = 0;
        virtual HRESULT STDMETHODCALLTYPE get__NewEnum (LPUNKNOWN* ppEnum)               = 0;
};
//---------------------------------------------------------------------------

class RegExp
{
    public:

        RegExp  (void);
        ~RegExp (void);

        bool SetPattern (const BSTRString &pattern);
        
        bool Execute (const BSTRString &source);
        bool Execute (const BSTRString &source, const BSTRString &pattern);

        bool Test (const BSTRString &source);
        bool Test (const BSTRString &source, const BSTRString &pattern);

        BSTRString Replace (const BSTRString &source, const BSTRString &replace);
        BSTRString Replace (const BSTRString &source, const BSTRString &pattern, const BSTRString &replace);

        DWORD MatchCount (void);

        BSTRString GetMatch (DWORD dwMatchIndex);

        DWORD SubMatchCount (DWORD dwMatchIndex);

        BSTRString GetSubMatch (DWORD dwMatchIndex, DWORD dwSubMatchIndex);

    private:

        IRegExp2*          m_regexp;
        IMatchCollection2* m_matches;
};
//---------------------------------------------------------------------------