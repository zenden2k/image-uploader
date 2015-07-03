#include "TextUtils.h"

#include "Core/3rdpart/pcreplusplus.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/3rdpart/htmlentities.h"
#include <stdio.h>

namespace IuTextUtils
{
    std::string BbCodeToHtml(const std::string& bbcode) {
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

    bool FileSaveContents(const std::string& fileName, const std::string& contents) {
        FILE * f = IuCoreUtils::fopen_utf8(fileName.c_str(), "wb");
        if ( !f ) {
            return false;
        }

        fwrite(contents.c_str(),1,contents.length(), f);
        fclose(f);
        return true;
    }

std::string DecodeHtmlEntities(const std::string& src)
{
    std::string res = src;
    decode_html_entities_utf8(&res[0], 0);
    res.resize(strlen(&res[0]));
    return res;
}

};