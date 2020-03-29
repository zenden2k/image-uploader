/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include <string>

namespace IuCoreUtils {

namespace CryptoUtils {
    std::string CalcMD5Hash(const void* data, size_t size);
    std::string CalcMD5HashFromString(const std::string &data);
    std::string CalcMD5HashFromFile(const std::string& filename);

    std::string CalcSHA1Hash(const void* data, size_t size);
    std::string CalcSHA1HashFromString(const std::string& data);
    std::string CalcSHA1HashFromFile(const std::string& filename);
    std::string CalcSHA1HashFromFileWithPrefix(const std::string& filename, const std::string& prefix, const std::string& postfix);

    std::string CalcHMACSHA1Hash(const std::string& key, const void* data, size_t size, bool base64);
    std::string CalcHMACSHA1HashFromString(const std::string& key, const std::string& data, bool base64);
    std::string Base64Encode(const std::string& data);
    std::string Base64Decode(const std::string& data);
    bool Base64EncodeFile(const std::string& fileName, std::string& result);
};

};

#endif // IU_CORE_UTILS_CRYPTOUTILS_H
