#include "WebUtils.h"

#include <Core/Utils/CoreUtils.h>
#include <Wininet.h>
#include <Core/3rdpart/pcreplusplus.h>
#include <Func/MyUtils.h>
#include <Func/WinUtils.h>

namespace WebUtils {

	bool DoesTextLookLikeUrl(CString& text) {
		/*URL_COMPONENTS urlComponents;
		TCHAR schemeBuffer[MAX_PATH] = _T("\0");
		TCHAR hostNameBuffer[MAX_PATH] = _T("\0");
		memset(&urlComponents, 0, sizeof(urlComponents));
		urlComponents.lpszHostName = hostNameBuffer;
		urlComponents.dwHostNameLength = ARRAY_SIZE(hostNameBuffer);
		urlComponents.lpszScheme = schemeBuffer;
		urlComponents.dwSchemeLength = ARRAY_SIZE(schemeBuffer);
		bool result = InternetCrackUrl(text, text.GetLength(), 0, &urlComponents);
		if ( result  && lstrlen(hostNameBuffer) ) {
			return true;
		}*/

		if ( text.Left(4) == _T("www.") ) {
			text = "http://" + text;
			return true;
		}
		std::string utf8Text = WCstringToUtf8(text);
		pcrepp::Pcre regexp2("^(http|https|ftp)://", "imcu");
		if ( regexp2.search(utf8Text, 0) ) { 
			if ( regexp2.get_match_start() == 0) {
				//text = "http://" + text;
				return true;
			}

		} 


		pcrepp::Pcre regexp("^[A-Za-z0-9-]+(\\.[A-Za-z0-9-]+)*(\\.[A-Za-z]{2,})", "imcu");


		if ( regexp.search(utf8Text, 0) ) { 
			if ( regexp.get_match_start() == 0) {
				text = "http://" + text;
				return true;
			}
			
		} 

		return false;
	}
}
