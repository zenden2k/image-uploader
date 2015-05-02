/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#ifndef IU_CORE_UTILS_CRYPTOUTILS_H
#define IU_CORE_UTILS_CRYPTOUTILS_H

#include <cstdio>
#include <string>
#include <vector>
#include "CoreTypes.h"

namespace IuCoreUtils {

namespace CryptoUtils {
    const std::string CalcMD5Hash(const void* data, size_t size);
    const std::string CalcMD5HashFromString(const std::string &data);
    const std::string CalcMD5HashFromFile(const std::string& filename);

    const std::string CalcSHA1Hash(const void* data, size_t size);
    const std::string CalcSHA1HashFromString(const std::string& data);
    const std::string CalcSHA1HashFromFile(const std::string& filename);

    const std::string CalcHMACSHA1Hash(const std::string& key, const void* data, size_t size, bool base64);
    const std::string CalcHMACSHA1HashFromString(const std::string& key, const std::string& data, bool base64);
};

};

#endif // IU_CORE_UTILS_CRYPTOUTILS_H
