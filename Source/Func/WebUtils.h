#ifndef FUNC_WEBUTILS_H
#define FUNC_WEBUTILS_H

#include "atlheaders.h"

namespace WebUtils {
    bool IsValidUrl(CString text);
    bool DoesTextLookLikeUrl(CString& text);
}

#endif