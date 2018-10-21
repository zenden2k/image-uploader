#include "TextUtils.h"

#include <stdio.h>

#include "Core/3rdpart/pcreplusplus.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/3rdpart/htmlentities.h"

//#include <codecvt>

namespace IuTextUtils
{
    std::string BbCodeToHtml(const std::string& bbcode) {
        std::string work = bbcode;
        
        pcrepp::Pcre reg2("\\[img\\](.+?)\\[\\/img\\]", "imc");
        std::string work2 = reg2.replace(work, "<img src=\"$1\" style=\"border:0\">");
        
        pcrepp::Pcre reg("\\[url=(.+?)\\](.+?)\\[/url\\]", "imc");
        std::string work3 = reg.replace(work2, "<a href=\"$1\" target=\"_blank\">$2</a>");
        

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

#define UNI_MAX_BMP   (UTF32)0x0000FFFF
#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END (UTF32)0xDBFF
#define UNI_SUR_LOW_START (UTF32)0xDC00
#define UNI_SUR_LOW_END (UTF32)0xDFFF
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD

#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF
static const int halfShift = 10; /* used for shifting by 10 bits */
static const UTF32 halfBase = 0x0010000UL;
static const UTF32 halfMask = 0x3FFUL;

ConversionResult ConvertUTF32toUTF16(const UTF32* source, const UTF32* sourceEnd, UTF16* target, UTF16* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    while (source < sourceEnd) {
        UTF32 ch;
        if (target >= targetEnd) {
            result = targetExhausted;
            break;
        }
        ch = *source++;
        if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
                if (flags == strictConversion) {
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    break;
                }
                else {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            }
            else {
                *target++ = (UTF16)ch; /* normal case */
            }
        }
        else if (ch > UNI_MAX_LEGAL_UTF32) {
            if (flags == strictConversion) {
                result = sourceIllegal;
            }
            else {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        }
        else {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            if (target + 1 >= targetEnd) {
                --source; /* Back up source pointer! */
                result = targetExhausted;
                break;
            }
            ch -= halfBase;
            *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_HIGH_START);
            *target++ = (UTF16)((ch & halfMask) + UNI_SUR_LOW_START);
        }
    }
    return result;
}

std::string DecodeHtmlEntities(const std::string& src)
{
    std::string res = src;
    decode_html_entities_utf8(&res[0], 0);
    res.resize(strlen(&res[0]));
    return res;
}

};