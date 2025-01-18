#ifndef IU_SERVERLISTTOOL_HELPERS_H
#define IU_SERVERLISTTOOL_HELPERS_H

#pragma once

#include "Core/Utils/CoreTypes.h"
#include "atlheaders.h"

namespace ServersListTool::Helpers {

struct MyFileInfo {
    int width;
    int height;
    CString mimeType;
    int size;
    MyFileInfo() {
        width = 0;
        height = 0;
        size = 0;
    }
};

CString GetFileInfo(CString fileName, MyFileInfo* mfi);

}

#endif
