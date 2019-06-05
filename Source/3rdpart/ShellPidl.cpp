
#include "ShellPidl.h"
#include "Core/Logging.h"

namespace {
    CComPtr<IMalloc> malloc_;
}

CComPtr<IMalloc> CShellPidl::getMalloc() {
    if (!malloc_) {
        ::SHGetMalloc(&malloc_);
    }
    return malloc_;
}

CShellPidl::CShellPidl() : m_pObj(nullptr)
{
    ::SHGetMalloc(&m_pMalloc);
}

CShellPidl::~CShellPidl()
{
    if (m_pMalloc)
        m_pMalloc->Release();
}

// does not include terminating zero
// NULL objects get a zero length
UINT CShellPidl::ILGetLength(LPCITEMIDLIST pidl)
{
    if (pidl == NULL)
        return 0;

    UINT length = 0, cb;
    do {
        cb = pidl->mkid.cb;
        pidl = (LPCITEMIDLIST)((LPBYTE)pidl + cb);
        //length += cb;
        length++;
    } while (cb != 0);

    return length;
}

LPCITEMIDLIST CShellPidl::ILGetNext(LPCITEMIDLIST pidl)
{
    if (pidl == NULL)
        return NULL;

    // Get the size of the specified item identifier. 
    int cb = pidl->mkid.cb;

    // If the size is zero, it is the end of the list. 
    if (cb == 0)
        return NULL;

    // Add cb to pidl (casting to increment by bytes). 
    pidl = (LPCITEMIDLIST)((LPBYTE)pidl + cb);

    // Return NULL if we reached the terminator, or a pidl otherwise. 
    return (pidl->mkid.cb == 0) ? NULL : pidl;
}

LPCITEMIDLIST CShellPidl::ILGetLast(LPCITEMIDLIST pidl)
{
    LPCITEMIDLIST pidlLast = pidl, tmp = ILGetNext(pidl);
    while (tmp != NULL) {
        pidlLast = tmp;
        tmp = ILGetNext(tmp);
    }

    return pidlLast;
}

LPITEMIDLIST CShellPidl::ILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    // check arguments
    if (pidl1 == NULL && pidl2 == NULL)
        return NULL;

    // Get the size of the resulting item identifier list
    UINT cb1 = ILGetLength(pidl1);
    UINT cb2 = ILGetLength(pidl2);

    // Allocate a new item identifier list
    LPITEMIDLIST pidlNew = (LPITEMIDLIST)(getMalloc()->Alloc(cb1 + cb2 + sizeof(USHORT)));
    LPITEMIDLIST pidlEnd = pidlNew;
    if (pidlNew != NULL) {
        // Copy the first item identifier list and terminating 0
        if (cb1 > 0) {
            CopyMemory(pidlEnd, pidl1, cb1);
            pidlEnd = (LPITEMIDLIST)((LPBYTE)pidlEnd + cb1);
        }

        // Copy the second item identifier list and terminating 0
        if (cb2 > 0) {
            CopyMemory(pidlEnd, pidl2, cb2);
            pidlEnd = (LPITEMIDLIST)((LPBYTE)pidlEnd + cb2);
        }

        // Append a terminating zero. 
        pidlEnd->mkid.cb = 0;
    }
    return pidlNew;
}

LPITEMIDLIST CShellPidl::ILCloneFirst(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlNew = NULL;

    if (pidl != NULL) {
        // Get the size of the first item identifier.
        int cb = pidl->mkid.cb;

        // Allocate a new item identifier list. 
        pidlNew = (LPITEMIDLIST)getMalloc()->Alloc(cb + sizeof(USHORT));

        if (pidlNew != NULL) {
            // Copy the specified item identifier. 
            CopyMemory(pidlNew, pidl, cb);

            // Append a terminating zero. 
            ((LPITEMIDLIST)((LPBYTE)pidlNew + cb))->mkid.cb = 0;
        }
    }
    return pidlNew;
}

LPITEMIDLIST CShellPidl::ILCloneParent(LPCITEMIDLIST pidl)
{
    // Get the size of the parent item identifier. 
    UINT cb = (UINT)ILGetLast(pidl) - (UINT)pidl;

    LPITEMIDLIST pidlNew = (LPITEMIDLIST)getMalloc()->Alloc(cb + sizeof(USHORT));

    if (pidlNew != NULL) {
        // Copy the specified item identifier. 
        CopyMemory(pidlNew, pidl, cb);

        // Append a terminating zero. 
        ((LPITEMIDLIST)((LPBYTE)pidlNew + cb))->mkid.cb = 0;
    }
    return pidlNew;
}

LPITEMIDLIST CShellPidl::ILFromPath(LPCTSTR pszPath, HWND hOwner) {
    LPITEMIDLIST pidlNew = nullptr;

    CComPtr<IShellFolder> desktop; // namespace root for parsing the path
    HRESULT hr = SHGetDesktopFolder(&desktop);

    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    desktop->ParseDisplayName(hOwner, nullptr, const_cast<LPWSTR>(pszPath), nullptr, &pidlNew, nullptr);

    return pidlNew;
}

CShellPidl::CShellPidl(LPCITEMIDLIST pidl)
{
    m_pMalloc = nullptr;
    m_pObj = ILClone(pidl);
}

CShellPidl::CShellPidl(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlRel)
{
    m_pMalloc = nullptr;
    m_pObj = ILCombine(pidlParent, pidlRel);
}

CShellPidl::CShellPidl(UINT nSpecialFolder, HWND hOwner)
{
    m_pMalloc = nullptr;
    SHGetSpecialFolderLocation(hOwner, nSpecialFolder, &m_pObj);
}

CShellPidl::CShellPidl(LPCTSTR pszPath, HWND hOwner)
{
    m_pObj = ILFromPath(pszPath, hOwner);
}

BOOL CShellPidl::IsRoot() const
{
    return (m_pObj != NULL) && (m_pObj->mkid.cb == 0);
}

CString CShellPidl::GetPath() const
{
    CString path;
    BOOL bSuccess = SHGetPathFromIDList(m_pObj, path.GetBuffer(MAX_PATH));
    path.ReleaseBuffer();
    if (!bSuccess)
        path.Empty();
    return path;
}

int CShellPidl::GetIconIndex(UINT uFlags) const
{
    SHFILEINFO sfi;
    ZeroMemory(&sfi, sizeof(SHFILEINFO));
    uFlags |= SHGFI_PIDL | SHGFI_SYSICONINDEX;
    SHGetFileInfo((LPCTSTR)m_pObj, 0, &sfi, sizeof(SHFILEINFO), uFlags);
    return sfi.iIcon;
}

void CShellPidl::Combine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    /*Destroy();
    Attach(ILCombine(pidl1, pidl2));*/
}

// get the first ancestor
void CShellPidl::CloneFirstParent(LPCITEMIDLIST pidl)
{
    /*Destroy();
    Attach(ILCloneFirst(pidl));*/
}

void CShellPidl::CloneFirstChild(LPCITEMIDLIST pidl)
{
    /*Destroy();
    Attach(ILClone(ILGetNext(pidl)));*/
}

LPCITEMIDLIST CShellPidl::GetFirstChild()
{
    return ILGetNext(m_pObj);
}

// get the immediate parent
void CShellPidl::CloneLastParent(LPCITEMIDLIST pidl)
{
    /*Destroy();
    Attach(ILCloneParent(pidl));*/
}

void CShellPidl::CloneLastChild(LPCITEMIDLIST pidl)
{
    /*Destroy();
    Attach(ILClone(ILGetLast(pidl)));*/
}

LPCITEMIDLIST CShellPidl::GetLastChild()
{
    return ILGetLast(m_pObj);
}

//////////////////////////////////////////////////////////////////////
// Static Functions

LPITEMIDLIST CShellPidl::ILClone(LPCITEMIDLIST pidl)
{
    return ILCombine(NULL, pidl);
}

/****************************************************************************
*
*  FUNCTION: DoTheMenuThing(HWND hwnd,
*                           LPSHELLFOLDER lpsfParent,
*                           LPITEMIDLIST  lpi,
*                           LPPOINT lppt)
*
*  PURPOSE: Displays a popup context menu, given a parent shell folder,
*           relative item id and screen location.
*
*  PARAMETERS:
*    hwnd       - Parent window handle
*    lpsfParent - Pointer to parent shell folder.
*    lpi        - Pointer to item id that is relative to lpsfParent
*    count       - Screen location of where to popup the menu.
*
*  RETURN VALUE:
*    Returns TRUE on success, FALSE on failure
*
****************************************************************************/
bool CShellPidl::MultiFileProperties(HWND hwnd, LPSHELLFOLDER lpsfParent, LPCITEMIDLIST*  lpi, int count) {
    bool    bSuccess = true;
    HRESULT hr;
    CComPtr<IDataObject> pDataObject;

    hr = lpsfParent->GetUIObjectOf(hwnd, count, lpi, IID_IDataObject, 0, reinterpret_cast<void**>(&pDataObject));

    if (SUCCEEDED(hr)) {
        hr = SHMultiFileProperties(pDataObject, 0);
        if (!SUCCEEDED(hr)) {
            CString f;
            f.Format(_T("hr=%lx"), hr);
            LOG(ERROR) << "InvokeCommand failed. " << f;
            bSuccess = FALSE;
        } 
    } else {
        CString f;
        f.Format(_T("GetUIObjectOf failed! hr=%lx"), hr);
        LOG(ERROR) << f;
        bSuccess = false;
    }
    return bSuccess;
}