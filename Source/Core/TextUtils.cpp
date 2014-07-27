#include "TextUtils.h"

#include <Core/3rdpart/pcreplusplus.h>
#include <Core/Utils/StringUtils.h>
#include <Core/Utils/CoreUtils.h>
namespace IuTextUtils
{
	Utf8String BbCodeToHtml(const Utf8String& bbcode) {
		std::string work = bbcode;

		
		pcrepp::Pcre reg2("\\[img\\](.+?)\\[/img\\]", "imc");
		work = reg2.replace(work, "<img src=\"$1\" border=\"0\">");
		pcrepp::Pcre reg("\\[url=(.+?)\\](.+?)\\[/url\\]", "imc");
		work = reg.replace(work, "<a href=\"$1\" target=\"_blank\">$2</a>");
		pcrepp::Pcre reg3("\\[url\\](.+?)\\[/url\\]", "imc");
		work = reg3.replace(work, "<a href=\"$1\" target=\"_blank\">$1</a>");
		work = IuStringUtils::Replace(work, "\r\n", "<br/>");
		work = IuStringUtils::Replace(work, "\n", "<br/>");
		return work;
	}

	bool FileSaveContents(const Utf8String& fileName, const Utf8String& contents) {
		FILE * f = IuCoreUtils::fopen_utf8(fileName.c_str(), "wb");
		if ( !f ) {
			return false;
		}

		fwrite(contents.c_str(),1,contents.length(), f);
		fclose(f);
		return true;
	}
};