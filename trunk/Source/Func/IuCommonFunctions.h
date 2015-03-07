#ifndef IU_FUNC_IUCOMMONFUNCTIONS_H
#define IU_FUNC_IUCOMMONFUNCTIONS_H

#include "atlheaders.h"

namespace IuCommonFunctions {
	const CString GetDataFolder();

	const CString GetVersion();

	BOOL CreateTempFolder();
	void ClearTempFolder(LPCTSTR folder);

	int GetNextImgFile(LPCTSTR folder, LPTSTR szBuffer, int nLength);

	CString FindDataFolder();


	extern CString IUTempFolder;

	extern CString IUCommonTempFolder;
};
#endif