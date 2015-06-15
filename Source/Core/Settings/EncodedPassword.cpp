#include "EncodedPassword.h"
#include "Core/Utils/CoreUtils.h"

CEncodedPassword::CEncodedPassword() {
}

CEncodedPassword::CEncodedPassword(const std::string& d) {
    data_ = d;
}

std::string CEncodedPassword::toEncodedData() const {
    std::string res;
    EncodeString(data_, res);
    return res;
}
/*
CEncodedPassword::operator const wchar_t*() {
    return data_;
}*/

#ifdef IU_WTL
CEncodedPassword& CEncodedPassword::operator=(const CString& text) {
    data_ = IuCoreUtils::WstringToUtf8((LPCTSTR)text);
    return *this;
}
#endif

void CEncodedPassword::DecodeString(const std::string& encodedString, std::string & Result, const char* code) {
    std::wstring szSource = IuCoreUtils::Utf8ToWstring(encodedString);
    wchar_t szDestination[1024];
    int br = strlen(code);
    int n = szSource.length() / 2;
    int j = 0;
    memset(szDestination, 0, sizeof(szDestination));

    int i;
    uint8_t* data = (uint8_t*)szDestination;
    *szDestination = 0;

    for (i = 0; i < n; i++) {
        if (j >= br)
            j = 0;

        BYTE b;
        b = static_cast<BYTE>((szSource[i * 2] - L'A') * 16 + (szSource[i * 2 + 1] - L'A'));
        b = b ^ code[j];
        data[i] = b;
        j++;
    }
    data[i] = 0;
    Result = IuCoreUtils::WstringToUtf8(szDestination);
}

void CEncodedPassword::EncodeString(const std::string& plainText, std::string& Result, const char* code)
{
    std::wstring szSource = IuCoreUtils::Utf8ToWstring(plainText);
    wchar_t szDestination[1024];
    int br = strlen(code);
    int n = szSource.length() * 2;
    int j = 0;

    uint8_t* data = (uint8_t*)&szSource[0];
    *szDestination = 0;
    for (int i = 0; i < n; i++) {
        if (j >= br)
            j = 0;

        BYTE b;
        b = data[i] ^ code[j];
        TCHAR bb[2] = { 0, 0 };
        bb[0] = L'A' + b / 16;
        lstrcat(szDestination, bb);
        bb[0] = L'A' + b % 16;
        lstrcat(szDestination, bb);
        j++;
    }
    Result = IuCoreUtils::WstringToUtf8(szDestination);
}
