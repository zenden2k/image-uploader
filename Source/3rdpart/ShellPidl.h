#ifndef SRC_3RDPART_SHELLPIDL_H
#define SRC_3RDPART_SHELLPIDL_H

#pragma once

#include "atlheaders.h"
#include <ShObjIdl.h>
#include <boost/smart_ptr/shared_ptr.hpp>

class CShellPidl {
public:
    static LPCITEMIDLIST ILGetLast(LPCITEMIDLIST pidl);
    // manage PIDLs
    static UINT ILGetLength(LPCITEMIDLIST pidl);
    static LPCITEMIDLIST ILGetNext(LPCITEMIDLIST pidl);
    static LPITEMIDLIST ILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    static LPITEMIDLIST ILClone(LPCITEMIDLIST pidl);
    static LPITEMIDLIST ILCloneFirst(LPCITEMIDLIST pidl);
    static LPITEMIDLIST ILCloneParent(LPCITEMIDLIST pidl);
    static LPITEMIDLIST ILFromPath(LPCTSTR pszPath, HWND hOwner = NULL);
    static bool MultiFileProperties(HWND hwnd, LPSHELLFOLDER lpsfParent, LPCITEMIDLIST*  lpi, int count);
protected:
    static CComPtr<IMalloc> getMalloc();
    IMalloc* m_pMalloc;
public:
    CShellPidl(LPCTSTR pszPath, HWND hOwner = NULL);
    CShellPidl(UINT nSpecialFolder, HWND hOwner = NULL);
    CShellPidl(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlRel);
    CShellPidl(LPCITEMIDLIST pidl);
    CShellPidl();
    virtual ~CShellPidl();

    int GetIconIndex(UINT uFlags = SHGFI_SMALLICON) const;
    BOOL IsRoot() const;

    void Combine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    void CloneLastParent(LPCITEMIDLIST pidl);
    void CloneLastChild(LPCITEMIDLIST pidl);
    void CloneFirstParent(LPCITEMIDLIST pidl);
    void CloneFirstChild(LPCITEMIDLIST pidl);

    LPCITEMIDLIST GetFirstChild();
    LPCITEMIDLIST GetLastChild();

    CString GetPath() const;

    LPITEMIDLIST m_pObj;
};




#endif // SRC_3RDPART_SHELLPIDL_H
