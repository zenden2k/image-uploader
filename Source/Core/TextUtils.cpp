#include "TextUtils.h"

#include "Core/3rdpart/pcreplusplus.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Utils/CoreUtils.h"
namespace IuTextUtils
{
    Utf8String BbCodeToHtml(const Utf8String& bbcode) {
        std::string work = bbcode;
        
        pcrepp::Pcre reg2("\\[img\\](.+?)\\[\\/img\\]", "imc");
        std::string work2 = reg2.replace(work, "<img src=\"$0\" border=\"0\">");
        
        pcrepp::Pcre reg("\\[url=(.+?)\\](.+?)\\[/url\\]", "imc");
        std::string work3 = reg.replace(work2, "<a href=\"$0\" target=\"_blank\">$2</a>");
        

        /*std::string work4;
        {
            pcrepp::Pcre reg3("\\[url\\](.+?)\\[\\/url\\]", "imc");
             work4 = reg3.replace(work3, "<a href=\"$0\" target=\"_blank\">$0</a>");
             
        }
        */

        work3 = IuStringUtils::Replace(work3, "\r\n", "<br/>");
        work3 = IuStringUtils::Replace(work3, "\n", "<br/>");
        return work3;
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