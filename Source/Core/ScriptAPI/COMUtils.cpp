#include "COMUtils.h"
#include <Core/Utils/CoreUtils.h>

namespace ScriptAPI {

std::string ComVariantToString(const CComVariant& src) {
	if ( src.vt == VT_BSTR ) {
		return IuCoreUtils::WstringToUtf8(src.bstrVal);
	}
	CComVariant vtBSTR;
	if ( SUCCEEDED( vtBSTR.ChangeType( VT_BSTR, &src ) ) )
	{
		return IuCoreUtils::WstringToUtf8(vtBSTR.bstrVal);
	}
	return std::string();
}

};