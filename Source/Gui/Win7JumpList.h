#ifndef IU_GUI_WIN7JUMPLIST_H
#define IU_GUI_WIN7JUMPLIST_H

#pragma once
#include "atlheaders.h"
#include <shobjidl.h>

class Win7JumpList {
public:
	Win7JumpList();
	~Win7JumpList();
    void DeleteJumpList();

protected:
	HRESULT CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, int iconId, IShellLink **ppsl);
	HRESULT AddCategoryToList(ICustomDestinationList *pcdl, IObjectArray *poaRemoved);
	void CreateJumpList();
	HRESULT AddTasksToList(ICustomDestinationList *pcdl);
	HRESULT CreateSeparatorLink(IShellLink **ppsl);
};

#endif
