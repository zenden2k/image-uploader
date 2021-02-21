/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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
#include <libbase64.h>
#include <Core/Upload/CommonTypes.h>

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
    return base64_decode(data);
}

bool Base64EncodeFile(const std::string& fileName, std::string& result) {
    FILE* f = fopen_utf8(fileName.c_str(), "rb");
    if (!f) {
        return false;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        return false;
    }
    long fsize = ftell(f);
    rewind(f);
    result.clear();

    if (!fsize) {
        return true;
    }
    size_t outlen = 0, bytes_read;
    size_t buffer_size = /*((4 * fsize / 3) + 3) & ~3*/ 4 * fsize / 3 + 5;
    const int read_buffer_size = 1024 * 1024;
    char* read_buffer = new char[read_buffer_size];
    result.resize(buffer_size, '\0');

    base64_state state;
    base64_stream_encode_init(&state, 0);
    char* encoded_cur = &result[0];
    size_t total_len = 0;
    while ((bytes_read = fread(read_buffer, 1, read_buffer_size, f)) > 0) {
        outlen = 0;
        base64_stream_encode(&state, read_buffer, bytes_read, encoded_cur, &outlen);
        encoded_cur += outlen;
        total_len += outlen;
    }

    base64_stream_encode_final(&state, encoded_cur, &outlen);
    total_len += outlen;
    result.resize(total_len);
    //encoded_cur += outlen;
    delete[] read_buffer;
    return true;
}

}
}
