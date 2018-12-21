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

#include "CryptoUtils.h"

#include "Core/3rdpart/base64.h"

namespace IuCoreUtils {
namespace CryptoUtils {

std::string Base64Encode(const std::string& data)
{
    /*std::string res;
    size_t outlen = (data.length() * 4 )/ 3+10;
    res.resize(outlen);
    base64_encode(data.data(), data.length(), &res[0], &outlen, BASE64_FORCE_PLAIN );
    res.resize(outlen);
    return res;*/
    return base64_encode((unsigned char const*)data.data(), data.length());
}

std::string Base64Decode(const std::string& data)
{
    return base64_decode(data.data());
}

}
}
