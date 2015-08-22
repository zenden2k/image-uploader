
#ifndef IU_CORE_UTILS_TEXTUTILS_H
#define IU_CORE_UTILS_TEXTUTILS_H

#include <string>
#include "CoreTypes.h"

namespace IuTextUtils{

/** 
Convert BBCode to HTML (not fully implemented)
**/
std::string BbCodeToHtml(const std::string& bbcode);
bool FileSaveContents(const std::string& fileName, const std::string& contents);
std::string DecodeHtmlEntities(const std::string& src);
typedef enum {
    conversionOK, /* conversion successful */
    sourceExhausted, /* partial character in source, but hit end */
    targetExhausted, /* insuff. room in target for conversion */
    sourceIllegal /* source sequence is illegal/malformed */

} ConversionResult;

typedef  uint32_t  UTF32; 
#ifdef _WIN32
typedef  wchar_t  UTF16;
#else
typedef  uint16_t  UTF16; 
#endif

typedef enum {
    strictConversion = 0,
    lenientConversion

} ConversionFlags;

ConversionResult ConvertUTF32toUTF16(const UTF32* source, const UTF32* sourceEnd, UTF16* target, UTF16* targetEnd, ConversionFlags flags);

};

#endif
