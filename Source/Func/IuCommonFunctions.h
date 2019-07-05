#ifndef IU_FUNC_IUCOMMONFUNCTIONS_H
#define IU_FUNC_IUCOMMONFUNCTIONS_H

#include "atlheaders.h"

namespace IuCommonFunctions {
    CString GetDataFolder();

    CString GetVersion();

    BOOL CreateTempFolder(CString& IUCommonTempFolder, CString& IUTempFolder);
    void ClearTempFolder(CString folder);

    int GetNextImgFile(LPCTSTR folder, CString& szBuffer);
    CString GenerateFileName(const CString &templateStr, int index, const CPoint& size, const CString& originalName = _T(""));
    bool IsImage(LPCTSTR szFileName);

    CString FindDataFolder();
};
#endif