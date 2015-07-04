
#ifndef IU_CORE_UTILS_TEXTUTILS_H
#define IU_CORE_UTILS_TEXTUTILS_H

#include <string>

namespace IuTextUtils{

/** 
Convert BBCode to HTML (not fully implemented)
**/
std::string BbCodeToHtml(const std::string& bbcode);
bool FileSaveContents(const std::string& fileName, const std::string& contents);
std::string DecodeHtmlEntities(const std::string& src);

};

#endif
