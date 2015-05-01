
#include <windows.h>
#include <winreg.h>
#include "Registry.h"
#include <string.h>
#include "Func/MyUtils.h"
#define CLASS_NAME_LENGTH 255

/* IMPORTANT NOTES ABOUT CREGISTRY:
    
    CRegistry never keeps a key open past the end of a function call.
    This is incase the application crashes before the next call to close
    the registry 
    
    KEY NAMES:
    Key names must not begin with a \ and only absolute strings are accepted
    
*/
 
CRegistry::CRegistry()
{
    m_wow64Flag = 0;
    m_hRootKey = HKEY_CURRENT_USER;
    m_bLazyWrite = TRUE;
    m_nLastError = ERROR_SUCCESS;
}

CRegistry::~CRegistry()
{
    ClearKey();
}

BOOL CRegistry::ClearKey()
{
    /* Call CloseKey to write the current key to the registry and close the 
    key. An application should not keep keys open any longer than necessary. 
    Calling CloseKey when there is no current key has no effect.*/

    m_strCurrentPath.Empty();
    m_hRootKey = HKEY_CURRENT_USER;
    m_bLazyWrite = TRUE;
    return TRUE;
}



BOOL CRegistry::SetRootKey(HKEY hRootKey)
{
    // sets the root key
    // make sure to set it to a valid key
    if (hRootKey != HKEY_CLASSES_ROOT &&
            hRootKey != HKEY_CURRENT_USER &&
            hRootKey != HKEY_LOCAL_MACHINE &&
            hRootKey != HKEY_USERS) return FALSE;

    m_hRootKey = hRootKey;
    return TRUE;
}


BOOL CRegistry::CreateKey(CString strKey)
{
    /* Use CreateKey to add a new key to the registry. 
        Key is the name of the key to create. Key must be 
        an absolute name. An absolute key 
        begins with a backslash (\) and is a subkey of 
        the root key. */

    ATLASSERT(strKey[0] != '\\');
    HKEY hKey;

    DWORD dwDisposition = 0;

    if (::RegCreateKeyEx(m_hRootKey, LPCTSTR(strKey), 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,
            &dwDisposition)    != ERROR_SUCCESS) return FALSE;
    
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    m_strCurrentPath = strKey;
    return TRUE;
}


BOOL CRegistry::DeleteKey(CString strKey)
{
    /* Call DeleteKey to remove a specified key and its associated data, 
    if any, from the registry. Returns FALSE is there are subkeys
    Subkeys must be explicitly deleted by separate calls to DeleteKey.
    DeleteKey returns True if key deletion is successful. On error, 
    DeleteKey returns False. */
    
    // need to open the key first with RegOpenKeyEx
//    ATLASSERT(FALSE); // not yet implemented
    ATLASSERT(strKey[0] != '\\');

    if (!KeyExists(strKey)) return TRUE;
    if (::RegDeleteKey(m_hRootKey, strKey) != ERROR_SUCCESS) return FALSE;
    return TRUE;
}



BOOL CRegistry::DeleteWithSubkeys(CString strKey)
{
    if (!KeyExists(strKey)) return TRUE;

    HKEY hKey;

    if( RegOpenKeyEx( m_hRootKey,
        strKey,
        0,
        KEY_ALL_ACCESS,
        &hKey) == ERROR_SUCCESS
        )
    {
        
        const int MAX_KEY_LENGTH = 256;
        const int MAX_VALUE_NAME = 1024;

        TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
        DWORD    cbName;                   // size of name string 
        TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
        DWORD    cchClassName = MAX_PATH;  // size of class string 
        DWORD    cSubKeys=0;               // number of subkeys 
        DWORD    cbMaxSubKey;              // longest subkey size 
        DWORD    cchMaxClass;              // longest class string 
        DWORD    cValues;              // number of values for key 
        DWORD    cchMaxValue;          // longest value name 
        DWORD    cbMaxValueData;       // longest value data 
        DWORD    cbSecurityDescriptor; // size of security descriptor 
        FILETIME ftLastWriteTime;      // last write time 

        DWORD i, retCode; 

        TCHAR  achValue[MAX_VALUE_NAME]; 
        DWORD cchValue = MAX_VALUE_NAME; 

        // Get the class name and the value count. 
        retCode = RegQueryInfoKey(
            hKey,                    // key handle 
            achClass,                // buffer for class name 
            &cchClassName,           // size of class string 
            NULL,                    // reserved 
            &cSubKeys,               // number of subkeys 
            &cbMaxSubKey,            // longest subkey size 
            &cchMaxClass,            // longest class string 
            &cValues,                // number of values for this key 
            &cchMaxValue,            // longest value name 
            &cbMaxValueData,         // longest value data 
            &cbSecurityDescriptor,   // security descriptor 
            &ftLastWriteTime);       // last write time 

    
        // Enumerate the subkeys, until RegEnumKeyEx fails.

        if (cSubKeys)
        {

            for (i=0; i<cSubKeys; i++) 
            { 
                cbName = MAX_KEY_LENGTH;
                retCode = RegEnumKeyEx(hKey, 0,
                    achKey, 
                    &cbName, 
                    NULL, 
                    NULL, 
                    NULL, 
                    &ftLastWriteTime); 
                
                if (retCode == ERROR_SUCCESS) 
                {
                    ::RegDeleteKey(hKey, achKey) ;

                }
            }
        } 
    }
    RegCloseKey(hKey);
    ::RegDeleteKey(m_hRootKey, strKey) ;
    return true;
}

BOOL CRegistry::DeleteValue(CString strName)
{
    /* Call DeleteValue to remove a specific data value 
        associated with the current key. Name is string 
        containing the name of the value to delete. Keys can contain 
        multiple data values, and every value associated with a key 
        has a unique name. */

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    HKEY hKey;
    LONG lResult;

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) return FALSE;

    lResult = ::RegDeleteValue(hKey, LPCTSTR(strName));
    ::RegCloseKey(hKey);

    if (lResult == ERROR_SUCCESS) return TRUE;
    return FALSE;
}


BOOL CRegistry::GetChildKeysNames(CString strKey, std::vector<CString>& outNames)
{
    if (!KeyExists(strKey)) return false;

    HKEY hKey;

    if( RegOpenKeyEx( m_hRootKey,
        strKey,
        0,
        KEY_ALL_ACCESS,
        &hKey) == ERROR_SUCCESS
        )
    {

        const int MAX_KEY_LENGTH = 256;
        const int MAX_VALUE_NAME = 1024;

        TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
        DWORD    cbName;                   // size of name string 
        TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
        DWORD    cchClassName = MAX_PATH;  // size of class string 
        DWORD    cSubKeys=0;               // number of subkeys 
        DWORD    cbMaxSubKey;              // longest subkey size 
        DWORD    cchMaxClass;              // longest class string 
        DWORD    cValues;              // number of values for key 
        DWORD    cchMaxValue;          // longest value name 
        DWORD    cbMaxValueData;       // longest value data 
        DWORD    cbSecurityDescriptor; // size of security descriptor 
        FILETIME ftLastWriteTime;      // last write time 

        DWORD i, retCode; 

        TCHAR  achValue[MAX_VALUE_NAME]; 
        DWORD cchValue = MAX_VALUE_NAME; 

        // Get the class name and the value count. 
        retCode = RegQueryInfoKey(
            hKey,                    // key handle 
            achClass,                // buffer for class name 
            &cchClassName,           // size of class string 
            NULL,                    // reserved 
            &cSubKeys,               // number of subkeys 
            &cbMaxSubKey,            // longest subkey size 
            &cchMaxClass,            // longest class string 
            &cValues,                // number of values for this key 
            &cchMaxValue,            // longest value name 
            &cbMaxValueData,         // longest value data 
            &cbSecurityDescriptor,   // security descriptor 
            &ftLastWriteTime);       // last write time 


        // Enumerate the subkeys, until RegEnumKeyEx fails.

        if (cSubKeys)
        {

            for (i=0; i<cSubKeys; i++) 
            { 
                cbName = MAX_KEY_LENGTH;
                retCode = RegEnumKeyEx(hKey, i,
                    achKey, 
                    &cbName, 
                    NULL, 
                    NULL, 
                    NULL, 
                    &ftLastWriteTime); 

                if (retCode == ERROR_SUCCESS) 
                {
                    outNames.push_back(achKey);

                }
            }
        } 
    }
    RegCloseKey(hKey);
    return true;
}

int CRegistry::GetDataSize(CString strValueName)
{
    /* Call GetDataSize to determine the size, in bytes, of 
    a data value associated with the current key. ValueName 
    is a string containing the name of the data value to query.
    On success, GetDataSize returns the size of the data value. 
    On failure, GetDataSize returns -1. */

    HKEY hKey;
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    LONG lResult;

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) return -1;

    DWORD dwSize = 1;
    lResult = ::RegQueryValueEx(hKey, LPCTSTR(strValueName),
        NULL, NULL, NULL, &dwSize);
    ::RegCloseKey(hKey);

    if (lResult != ERROR_SUCCESS) return -1;
    return (int)dwSize;
}

DWORD CRegistry::GetDataType(CString strValueName)
{
    HKEY hKey;
    ATLASSERT(m_strCurrentPath.GetLength() > 0);

    m_nLastError = ::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_QUERY_VALUE, &hKey);

    if (m_nLastError != ERROR_SUCCESS) return 0;

    DWORD dwType = 1;
    m_nLastError = ::RegQueryValueEx(hKey, LPCTSTR(strValueName),
        NULL, &dwType, NULL, NULL);
    ::RegCloseKey(hKey);        

    if (m_nLastError == ERROR_SUCCESS) return dwType;

    return 0;
}



int CRegistry::GetSubKeyCount()
{
    /* Call this function to determine the number of subkeys.
        the function returns -1 on error */
    HKEY hKey;
    ATLASSERT(m_strCurrentPath.GetLength() > 0);

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) return -1;

    LONG lResult;
    DWORD dwSubKeyCount, dwValueCount, dwClassNameLength,
        dwMaxSubKeyName, dwMaxValueName, dwMaxValueLength;
    FILETIME ftLastWritten;

    _TCHAR szClassBuffer[CLASS_NAME_LENGTH];
        
    dwClassNameLength = CLASS_NAME_LENGTH;
    lResult = ::RegQueryInfoKey(hKey, szClassBuffer, &dwClassNameLength,
        NULL, &dwSubKeyCount, &dwMaxSubKeyName, NULL, &dwValueCount,
        &dwMaxValueName, &dwMaxValueLength, NULL, &ftLastWritten);
                
    ::RegCloseKey(hKey);
    if (lResult != ERROR_SUCCESS) return -1;

    return (int)dwSubKeyCount;
}


int CRegistry::GetValueCount()
{
    /* Call this function to determine the number of subkeys.
        the function returns -1 on error */
    HKEY hKey;
    ATLASSERT(m_strCurrentPath.GetLength() > 0);

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) return -1;

    LONG lResult;
    DWORD dwSubKeyCount, dwValueCount, dwClassNameLength,
        dwMaxSubKeyName, dwMaxValueName, dwMaxValueLength;
    FILETIME ftLastWritten;

    _TCHAR szClassBuffer[CLASS_NAME_LENGTH];
        
    dwClassNameLength = CLASS_NAME_LENGTH;
    lResult = ::RegQueryInfoKey(hKey, szClassBuffer, &dwClassNameLength,
        NULL, &dwSubKeyCount, &dwMaxSubKeyName, NULL, &dwValueCount,
        &dwMaxValueName, &dwMaxValueLength, NULL, &ftLastWritten);
                
    ::RegCloseKey(hKey);
    if (lResult != ERROR_SUCCESS) return -1;

    return (int)dwValueCount;
}


BOOL CRegistry::KeyExists(CString strKey, HKEY hRootKey)
{
    /* Call KeyExists to determine if a key of a specified name exists.
         Key is the name of the key for which to search. */

    ATLASSERT(strKey[0] != '\\');
    HKEY hKey;

    if (hRootKey == NULL) hRootKey = m_hRootKey;
    
    LONG lResult = ::RegOpenKeyEx(hRootKey, LPCTSTR(strKey), 0,
        KEY_ALL_ACCESS, &hKey);
    ::RegCloseKey(hKey);
    if (lResult == ERROR_SUCCESS) return TRUE;
    return FALSE;
}

BOOL CRegistry::SetKey(CString strKey, BOOL bCanCreate)
{
    /* Call SetKey to make a specified key the current key. Key is the 
        name of the key to open. If Key is null, the CurrentKey property 
        is set to the key specified by the RootKey property.

        CanCreate specifies whether to create the specified key if it does 
        not exist. If CanCreate is True, the key is created if necessary.

        Key is opened or created with the security access value KEY_ALL_ACCESS. 
        OpenKey only creates non-volatile keys, A non-volatile key is stored in 
        the registry and is preserved when the system is restarted. 

        OpenKey returns True if the key is successfully opened or created */

    ATLASSERT(strKey[0] != '\\');
    HKEY hKey;
    DWORD access = bCanCreate? KEY_ALL_ACCESS: KEY_READ;

    // close the current key if it is open
    if (strKey.GetLength() == 0)
    {
        m_strCurrentPath.Empty();
        return TRUE;
    }

    DWORD dwDisposition;
    if (bCanCreate) // open the key with RegCreateKeyEx
    {
        if (::RegCreateKeyEx(m_hRootKey, LPCTSTR(strKey), 0, NULL, 
            REG_OPTION_NON_VOLATILE, access|m_wow64Flag, NULL, &hKey,
                &dwDisposition) != ERROR_SUCCESS) return FALSE;
        m_strCurrentPath = strKey;
        if (!m_bLazyWrite) ::RegFlushKey(hKey);
        ::RegCloseKey(hKey);    
        return TRUE;
    }

    // otherwise, open the key without creating
    // open key requires no initial slash
    m_nLastError = ::RegOpenKeyEx(m_hRootKey, LPCTSTR(strKey), 0,
        access|m_wow64Flag, &hKey);
    if (m_nLastError != ERROR_SUCCESS) return FALSE;
    m_strCurrentPath = strKey;
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return TRUE;
}


BOOL CRegistry::ValueExists(CString strName)
{
    /* Call ValueExists to determine if a particular key exists in 
        the registry. Calling Value Exists is especially useful before 
        calling other TRegistry methods that operate only on existing keys.

        Name is the name of the data value for which to check.
    ValueExists returns True if a match if found, False otherwise. */

    HKEY hKey;
    LONG lResult;
    ATLASSERT(m_strCurrentPath.GetLength() > 0);

    
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) return FALSE;

    lResult = ::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        NULL, NULL, NULL);
    ::RegCloseKey(hKey);

    if (lResult == ERROR_SUCCESS) return TRUE;
    return FALSE;
}


void CRegistry::RenameValue(CString strOldName, CString strNewName)
{
    /* Call RenameValue to change the name of a data value associated 
        with the current key. OldName is a string containing the current 
        name of the data value. NewName is a string containing the replacement 
        name for the data value.
        
        If OldName is the name of an existing data value for the current key, 
        and NewName is not the name of an existing data value for the current 
        key, RenameValue changes the data value name as specified. Otherwise 
        the current name remains unchanged.
    */
    ATLASSERT(FALSE); // functionality not yet implemented
}


double CRegistry::ReadFloat(CString strName, double fDefault)
{
    /* Call ReadFloat to read a float value from a specified 
        data value associated with the current key. Name is the name 
        of the data value to read.
        
        If successful, ReadFloat returns a double value. 
        On error, an exception is raised, and the value returned by 
        this function should be discarded. */

    DWORD dwType = REG_BINARY;
    double d;
    DWORD dwSize = sizeof(d);
    HKEY hKey;

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ, &hKey) != ERROR_SUCCESS) return fDefault;

    if (::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)&d, &dwSize) != ERROR_SUCCESS) d = fDefault;
    ::RegCloseKey(hKey);    
    return d;
}

CString CRegistry::ReadString(CString strName, CString strDefault)
{
    DWORD dwType = REG_SZ;
    DWORD dwSize = 255;
    BOOL bSuccess = TRUE;
    _TCHAR sz[255];
    HKEY hKey;
    
                                 
    ATLASSERT(m_strCurrentPath.GetLength() > 0);

    // make sure it is the proper type
    dwType = GetDataType(strName);
    
    if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
    {
        return strDefault;
    }

    m_nLastError = ::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ|m_wow64Flag, &hKey);
    if (m_nLastError != ERROR_SUCCESS) return strDefault;

    m_nLastError = ::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)sz, &dwSize);
    if (m_nLastError != ERROR_SUCCESS) bSuccess = FALSE;
    ::RegCloseKey(hKey);    
    
    if (!bSuccess) return strDefault;
    return CString((LPCTSTR)sz);
}

DWORD CRegistry::ReadDword(CString strName, DWORD dwDefault)
{
    DWORD dwType = REG_DWORD;
    DWORD dw;
    DWORD dwSize = sizeof(dw);
    HKEY hKey;

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ, &hKey) != ERROR_SUCCESS) return dwDefault;

    if (::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)&dw, &dwSize) != ERROR_SUCCESS) dw = dwDefault;
    ::RegCloseKey(hKey);    
    return dw;
}



int CRegistry::ReadInt(CString strName, int nDefault)
{
    DWORD dwType = REG_BINARY;
    int n;
    DWORD dwSize = sizeof(n);
    HKEY hKey;

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ, &hKey) != ERROR_SUCCESS) return nDefault;

    if (::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)&n, &dwSize) != ERROR_SUCCESS) n = nDefault;
    ::RegCloseKey(hKey);    
    return n;
}

bool CRegistry::ReadBool(CString strName, bool bDefault)
{
    DWORD dwType = REG_DWORD;
    BOOL b;
    DWORD dwSize = sizeof(b);
    HKEY hKey;

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ|m_wow64Flag, &hKey) != ERROR_SUCCESS) return bDefault;

    if (::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)&b, &dwSize) != ERROR_SUCCESS) b = bDefault;
    ::RegCloseKey(hKey);    
    return b != FALSE;
}


COLORREF CRegistry::ReadColor(CString strName, COLORREF rgbDefault)
{
    DWORD dwType = REG_BINARY;
    COLORREF rgb;
    DWORD dwSize = sizeof(rgb);
    HKEY hKey;

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ, &hKey) != ERROR_SUCCESS) return rgbDefault;

    if (::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)&rgb, &dwSize) != ERROR_SUCCESS) rgb = rgbDefault;
    ::RegCloseKey(hKey);    
    return rgb;
}

BOOL CRegistry::ReadFont(CString strName, CFont* pFont)
{
    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(LOGFONT);
    BOOL bSuccess = TRUE;
    HKEY hKey;
    LOGFONT lf;

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ, &hKey) != ERROR_SUCCESS) return FALSE;

    if (::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)&lf, &dwSize) != ERROR_SUCCESS) bSuccess = FALSE;
    ::RegCloseKey(hKey);    
    if (bSuccess)
    {
        pFont->Detach();
        pFont->CreateFontIndirect(&lf);
    }
    return bSuccess;
}


BOOL CRegistry::ReadPoint(CString strName, CPoint* pPoint)
{
    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(CPoint);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ, &hKey) != ERROR_SUCCESS) return FALSE;

    if (::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)pPoint, &dwSize) != ERROR_SUCCESS) bSuccess = FALSE;
    ::RegCloseKey(hKey);    
    return bSuccess;
}

BOOL CRegistry::ReadSize(CString strName, CSize* pSize)
{
    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(CSize);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ, &hKey) != ERROR_SUCCESS) return FALSE;

    if (::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)pSize, &dwSize) != ERROR_SUCCESS) bSuccess = FALSE;
    ::RegCloseKey(hKey);    
    return bSuccess;
}

BOOL CRegistry::ReadRect(CString strName, CRect* pRect)
{
    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(CRect);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_READ, &hKey) != ERROR_SUCCESS) return FALSE;

    if (::RegQueryValueEx(hKey, LPCTSTR(strName), NULL,
        &dwType, (LPBYTE)pRect, &dwSize) != ERROR_SUCCESS) bSuccess = FALSE;
    ::RegCloseKey(hKey);    
    return bSuccess;
}




BOOL CRegistry::WriteBool(CString strName, BOOL bValue)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;
    DWORD value = bValue;
    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_DWORD, (LPBYTE)&value, sizeof(value))
         != ERROR_SUCCESS) bSuccess = FALSE;
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}



BOOL CRegistry::WriteString(CString strName, CString strValue)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;
    _TCHAR sz[255];

    if (strValue.GetLength() > 254) return FALSE;

#ifdef _UNICODE
    wcscpy(sz, LPCTSTR(strValue));
#else
    strcpy(sz, LPCTSTR(strValue));
#endif

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
#ifdef _UNICODE
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_SZ, (LPBYTE)sz, (wcslen(sz) + 1)*sizeof(TCHAR))
         != ERROR_SUCCESS) bSuccess = FALSE;
#else
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_SZ, (LPBYTE)sz, strlen(sz) + 1)
         != ERROR_SUCCESS) bSuccess = FALSE;
#endif
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}


BOOL CRegistry::WriteFloat(CString strName, double fValue)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_BINARY, (LPBYTE)&fValue, sizeof(fValue))
         != ERROR_SUCCESS) bSuccess = FALSE;
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}

BOOL CRegistry::WriteInt(CString strName, int nValue)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_BINARY, (LPBYTE)&nValue, sizeof(nValue))
         != ERROR_SUCCESS) bSuccess = FALSE;
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}

BOOL CRegistry::WriteDword(CString strName, DWORD dwValue)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue))
         != ERROR_SUCCESS) bSuccess = FALSE;
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}

BOOL CRegistry::WriteColor(CString strName, COLORREF rgbValue)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_BINARY, (LPBYTE)&rgbValue, sizeof(rgbValue))
         != ERROR_SUCCESS) bSuccess = FALSE;
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}


BOOL CRegistry::WriteFont(CString strName, CFont* pFont)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    LOGFONT lf;
    pFont->GetLogFont(&lf);

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_BINARY, (LPBYTE)&lf, sizeof(lf))
         != ERROR_SUCCESS) bSuccess = FALSE;
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}


BOOL CRegistry::WritePoint(CString strName, CPoint* pPoint)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_BINARY, (LPBYTE)pPoint, sizeof(CPoint))
         != ERROR_SUCCESS) bSuccess = FALSE;
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}


BOOL CRegistry::WriteSize(CString strName, CSize* pSize)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_BINARY, (LPBYTE)pSize, sizeof(CSize))
         != ERROR_SUCCESS) bSuccess = FALSE;
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}

BOOL CRegistry::WriteRect(CString strName, CRect* pRect)
{
    ATLASSERT(m_strCurrentPath.GetLength() > 0);
    BOOL bSuccess = TRUE;
    HKEY hKey;

    if (::RegOpenKeyEx(m_hRootKey, LPCTSTR(m_strCurrentPath), 0,
        KEY_WRITE, &hKey) != ERROR_SUCCESS) return FALSE;
    
    if (::RegSetValueEx(hKey, LPCTSTR(strName), 0,
        REG_BINARY, (LPBYTE)pRect, sizeof(CRect))
         != ERROR_SUCCESS) bSuccess = FALSE;
        
    if (!m_bLazyWrite) ::RegFlushKey(hKey);
    ::RegCloseKey(hKey);
    return bSuccess;
}

void CRegistry::SetWOW64Flag(DWORD flag)
{
    m_wow64Flag = flag;
}