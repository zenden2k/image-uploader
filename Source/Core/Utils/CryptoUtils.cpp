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

#include <cmath>
#include <libbase64.h>

#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "Core/Upload/CommonTypes.h"

namespace IuCoreUtils::CryptoUtils {

std::string Base64Encode(const std::string& data)
{
    std::string res;
    size_t outlen = ((4 * data.length() / 3) + 3) & ~3;
    res.resize(outlen);
    base64_encode(data.data(), data.length(), &res[0], &outlen, 0);
    res.resize(outlen);
    return res;
}

std::string Base64EncodeRaw(const char* bytes, unsigned int len) {
    std::string res;
    size_t outlen = ((4 * len / 3) + 3) & ~3;
    res.resize(outlen);
    base64_encode(bytes, len, &res[0], &outlen, 0);
    res.resize(outlen);
    return res;
}
std::string Base64Decode(const std::string& data)
{
    std::string res;
    auto outlen = static_cast<size_t>(std::ceil(data.length() * 3 / 4.0));
    res.resize(outlen);
    base64_decode(data.data(), data.length(), &res[0], &outlen, 0);
    res.resize(outlen);
    return res;
}

bool Base64EncodeFile(const std::string& fileName, std::string& result) {
    FILE* f = FopenUtf8(fileName.c_str(), "rb");
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
    auto read_buffer = std::make_unique<char[]>(read_buffer_size);
    result.resize(buffer_size, '\0');

    base64_state state;
    base64_stream_encode_init(&state, 0);
    char* encoded_cur = &result[0];
    size_t total_len = 0;
    while ((bytes_read = fread(read_buffer.get(), 1, read_buffer_size, f)) > 0) {
        outlen = 0;
        base64_stream_encode(&state, read_buffer.get(), bytes_read, encoded_cur, &outlen);
        encoded_cur += outlen;
        total_len += outlen;
    }

    base64_stream_encode_final(&state, encoded_cur, &outlen);
    total_len += outlen;
    result.resize(total_len);
    //encoded_cur += outlen;
    return true;
}

std::string DecryptAES(const std::string& base64cipher, const std::string& keyStr) {
    const std::vector<unsigned char> key(keyStr.begin(), keyStr.end());
    if (key.size() != 16) {
        throw std::runtime_error("Key must be 16 bytes for AES-128");
    }

    std::string base64Str = Base64Decode(base64cipher);
    std::vector<unsigned char> cipherData(base64Str.begin(), base64Str.end());

    if (cipherData.size() <= 16) {
        throw std::runtime_error("Ciphertext too short (no IV)");
    }

    std::vector<unsigned char> iv(cipherData.begin(), cipherData.begin() + 16);
    std::vector<unsigned char> encrypted(cipherData.begin() + 16, cipherData.end());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    }

    int len;
    std::vector<unsigned char> plaintext(encrypted.size() + 16);

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key.data(), iv.data())) {
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }

    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, encrypted.data(), encrypted.size())) {
        throw std::runtime_error("EVP_DecryptUpdate failed");
    }

    int plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) {
        throw std::runtime_error("EVP_DecryptFinal_ex failed");
    }

    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return std::string(plaintext.begin(), plaintext.begin() + plaintext_len);
}

}
