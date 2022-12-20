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

#include "CoreUtils.h"
#include <cstdio>
#include <string>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include "Core/3rdpart/base64.h"

namespace IuCoreUtils {

std::string CryptoUtils::CalcMD5Hash(const void* data, size_t size)
{
    std::string result;
    MD5_CTX context;

    MD5_Init(&context);
    MD5_Update(&context, data, size);

    unsigned char buff[16] = "";    

    MD5_Final(buff, &context);

    for(int i=0;i<16; i++)
    {
        char temp[5];
        sprintf(temp, "%02x",buff[i]);
        result += temp;
    }
    return result;
}

std::string CryptoUtils::CalcMD5HashFromString(const std::string& data)
{
    std::string result;
    MD5_CTX context;

    MD5_Init(&context);
    MD5_Update(&context, (unsigned char*)data.c_str(), data.length());

    unsigned char buff[16] = "";    

    MD5_Final(buff, &context);

    for(int i=0;i<16; i++)
    {
        char temp[5];
        sprintf(temp, "%02x",buff[i]);
        result += temp;
    }
    return result;
}

std::string CryptoUtils::CalcMD5HashFromFile(const std::string& filename) {
    std::string result;
    MD5_CTX context;

    MD5_Init(&context);
    FILE* f = IuCoreUtils::FopenUtf8( filename.c_str(), "rb" );
    if (f) {
        unsigned char buf[4096];
        while ( !feof(f) ) {
            size_t bytesRead = fread( buf, 1, sizeof(buf), f );
            if (  bytesRead == 0 ) {
                break;
            }
            MD5_Update(&context, (unsigned char*)buf, bytesRead);
        }
        unsigned char buff[16] = "";

        MD5_Final(buff, &context);

        fclose(f);

        for (int i = 0; i < 16; i++) {
            char temp[5];
            sprintf( temp, "%02x", buff[i] );
            result += temp;
        }
    }

    return result;
}

std::string CryptoUtils::CalcSHA1Hash(const void* data, size_t size) {
    const int HashSize = 20;
    std::string result;
    SHA_CTX context;

    SHA1_Init( &context );
    SHA1_Update( &context, data, size );

    unsigned char buff[HashSize] = "";    

    SHA1_Final( buff, &context);

    for ( int i = 0; i < HashSize; i++) {
        char temp[5];
        sprintf(temp, "%02x",buff[i]);
        result += temp;
    }
    return result;
}

std::string CryptoUtils::CalcHMACSHA1Hash(const std::string& key, const void* data, size_t size, bool base64) {
    unsigned char* digest;

    // Using sha1 hash engine here.
    // You may use other hash engines. e.g EVP_md5(), EVP_sha224, EVP_sha512, etc
    digest = HMAC(EVP_sha1(), key.c_str(), key.length(), (unsigned char*)data, size, NULL, NULL);    

    char mdString[50]="";
    for(int i = 0; i < 20; i++) {
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
    }
    if ( base64 ) {
        return  base64_encode(digest,20);
    }

    return mdString;
}

std::string CryptoUtils::CalcHMACSHA1HashFromString(const std::string& key, const std::string& data, bool base64) {
    return CalcHMACSHA1Hash( key, data.c_str(), data.size(), base64 );
}

std::string CryptoUtils::CalcSHA1HashFromString(const std::string& data) {
    return CalcSHA1Hash( data.c_str(), data.size() );
}

std::string CryptoUtils::CalcSHA1HashFromFileWithPrefix(const std::string& filename, const std::string& prefix, const std::string& postfix) {
    const int HashSize = 20;
    std::string result;
    SHA_CTX context;

    SHA1_Init(&context);
    FILE* f = IuCoreUtils::FopenUtf8( filename.c_str(), "rb" );

    if (f) {
        if (!prefix.empty()) {
            SHA1_Update(&context, (unsigned char*)prefix.data(), prefix.length());
        }
        unsigned char buf[4096];
        while ( !feof(f) ) {
            size_t bytesRead = fread(buf, 1, sizeof(buf), f);

            SHA1_Update(&context, (unsigned char*)buf, bytesRead);
        }
        unsigned char buff[HashSize] = "";
        if (!postfix.empty()) {
            SHA1_Update(&context, (unsigned char*)postfix.data(), postfix.length());
        }
        SHA1_Final(buff, &context);

        fclose(f);

        for (int i = 0; i < HashSize; i++) {
            char temp[5];
            sprintf(temp, "%02x", buff[i]);
            result += temp;
        }
    }
    return result;
}

std::string CryptoUtils::CalcSHA1HashFromFile(const std::string& filename) {
    return CalcSHA1HashFromFileWithPrefix(filename, "", "");
}

}; // end of namespace IuCoreUtils
