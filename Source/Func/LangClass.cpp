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

#include "LangClass.h"

#include <cstdio>
#include <filesystem>
#include <sstream>

#ifndef IU_SHELLEXT
    #include "Core/Utils/CoreUtils.h"
#endif

namespace {

    std::unordered_map<std::string, std::string> localeNames = {
        {"af_NA", "Afrikaans (Namibia)"},
    {"af_ZA", "Afrikaans (South Africa)"},
    {"af", "Afrikaans"},
    {"ak_GH", "Akan (Ghana)"},
    {"ak", "Akan"},
    {"sq_AL", "Albanian (Albania)"},
    {"sq", "Albanian"},
    {"am_ET", "Amharic (Ethiopia)"},
    {"am", "Amharic"},
    {"ar_DZ", "Arabic (Algeria)"},
    {"ar_BH", "Arabic (Bahrain)"},
    {"ar_EG", "Arabic (Egypt)"},
    {"ar_IQ", "Arabic (Iraq)"},
    {"ar_JO", "Arabic (Jordan)"},
    {"ar_KW", "Arabic (Kuwait)"},
    {"ar_LB", "Arabic (Lebanon)"},
    {"ar_LY", "Arabic (Libya)"},
    {"ar_MA", "Arabic (Morocco)"},
    {"ar_OM", "Arabic (Oman)"},
    {"ar_QA", "Arabic (Qatar)"},
    {"ar_SA", "Arabic (Saudi Arabia)"},
    {"ar_SD", "Arabic (Sudan)"},
    {"ar_SY", "Arabic (Syria)"},
    {"ar_TN", "Arabic (Tunisia)"},
    {"ar_AE", "Arabic (United Arab Emirates)"},
    {"ar_YE", "Arabic (Yemen)"},
    {"ar", "Arabic"},
    {"hy_AM", "Armenian (Armenia)"},
    {"hy", "Armenian"},
    {"as_IN", "Assamese (India)"},
    {"as", "Assamese"},
    {"asa_TZ", "Asu (Tanzania)"},
    {"asa", "Asu"},
    {"az_Cyrl", "Azerbaijani (Cyrillic)"},
    {"az_Cyrl_AZ", "Azerbaijani (Cyrillic, Azerbaijan)"},
    {"az_Latn", "Azerbaijani (Latin)"},
    {"az_Latn_AZ", "Azerbaijani (Latin, Azerbaijan)"},
    {"az", "Azerbaijani"},
    {"bm_ML", "Bambara (Mali)"},
    {"bm", "Bambara"},
    {"eu_ES", "Basque (Spain)"},
    {"eu", "Basque"},
    {"be_BY", "Belarusian (Belarus)"},
    {"be", "Belarusian"},
    {"bem_ZM", "Bemba (Zambia)"},
    {"bem_ZM", "(Zambia)"},
    {"bem", "Bemba"},
    {"bez_TZ", "Bena (Tanzania)"},
    {"bez", "Bena"},
    {"bn_BD", "Bengali (Bangladesh)"},
    {"bn_IN", "Bengali (India)"},
    {"bn", "Bengali"},
    {"bs_BA", "Bosnian (Bosnia and Herzegovina)"},
    {"bs", "Bosnian"},
    {"bg_BG", "Bulgarian (Bulgaria)"},
    {"bg", "Bulgarian"},
    {"my_MM", "Burmese (Myanmar [Burma])"},
    {"my", "Burmese"},
    {"yue_Hant_HK", "Cantonese (Traditional, Hong Kong SAR China)"},
    {"ca_ES", "Catalan (Spain)"},
    {"ca", "Catalan"},
    {"tzm_Latn", "Central Morocco Tamazight (Latin)"},
    {"tzm_Latn_MA", "Central Morocco Tamazight (Latin, Morocco)"},
    {"tzm", "Central Morocco Tamazight"},
    {"chr_US", "Cherokee (United States)"},
    {"chr", "Cherokee"},
    {"cgg_UG", "Chiga (Uganda)"},
    {"cgg", "Chiga"},
    {"zh_Hans", "Chinese (Simplified Han)"},
    {"zh_Hans_CN", "Chinese (Simplified Han, China)"},
    {"zh_Hans_HK", "Chinese (Simplified Han, Hong Kong SAR China)"},
    {"zh_Hans_MO", "Chinese (Simplified Han, Macau SAR China)"},
    {"zh_Hans_SG", "Chinese (Simplified Han, Singapore)"},
    {"zh_Hant", "Chinese (Traditional Han)"},
    {"zh_Hant_HK", "Chinese (Traditional Han, Hong Kong SAR China)"},
    {"zh_Hant_MO", "Chinese (Traditional Han, Macau SAR China)"},
    {"zh_Hant_TW", "Chinese (Traditional Han, Taiwan)"},
    {"zh", "Chinese"},
    {"kw_GB", "Cornish (United Kingdom)"},
    {"kw", "Cornish"},
    {"hr_HR", "Croatian (Croatia)"},
    {"hr", "Croatian"},
    {"cs_CZ", "Czech (Czech Republic)"},
    {"cs", "Czech"},
    {"da_DK", "Danish (Denmark)"},
    {"da", "Danish"},
    {"nl_BE", "Dutch (Belgium)"},
    {"nl_NL", "Dutch (Netherlands)"},
    {"nl", "Dutch"},
    {"ebu_KE", "Embu (Kenya)"},
    {"ebu", "Embu"},
    {"en_AS", "English (American Samoa)"},
    {"en_AU", "English (Australia)"},
    {"en_BE", "English (Belgium)"},
    {"en_BZ", "English (Belize)"},
    {"en_BW", "English (Botswana)"},
    {"en_CA", "English (Canada)"},
    {"en_GU", "English (Guam)"},
    {"en_HK", "English (Hong Kong SAR China)"},
    {"en_IN", "English (India)"},
    {"en_IE", "English (Ireland)"},
    {"en_IL", "English (Israel)"},
    {"en_JM", "English (Jamaica)"},
    {"en_MT", "English (Malta)"},
    {"en_MH", "English (Marshall Islands)"},
    {"en_MU", "English (Mauritius)"},
    {"en_NA", "English (Namibia)"},
    {"en_NZ", "English (New Zealand)"},
    {"en_MP", "English (Northern Mariana Islands)"},
    {"en_PK", "English (Pakistan)"},
    {"en_PH", "English (Philippines)"},
    {"en_SG", "English (Singapore)"},
    {"en_ZA", "English (South Africa)"},
    {"en_TT", "English (Trinidad and Tobago)"},
    {"en_UM", "English (U.S. Minor Outlying Islands)"},
    {"en_VI", "English (U.S. Virgin Islands)"},
    {"en_GB", "English (United Kingdom)"},
    {"en_US", "English (United States)"},
    {"en_ZW", "English (Zimbabwe)"},
    {"en", "English"},
    {"eo", "Esperanto"},
    {"et_EE", "Estonian (Estonia)"},
    {"et", "Estonian"},
    {"ee_GH", "Ewe (Ghana)"},
    {"ee_TG", "Ewe (Togo)"},
    {"ee", "Ewe"},
    {"fo_FO", "Faroese (Faroe Islands)"},
    {"fo", "Faroese"},
    {"fil_PH", "Filipino (Philippines)"},
    {"fil", "Filipino"},
    {"fi_FI", "Finnish (Finland)"},
    {"fi", "Finnish"},
    {"fr_BE", "French (Belgium)"},
    {"fr_BJ", "French (Benin)"},
    {"fr_BF", "French (Burkina Faso)"},
    {"fr_BI", "French (Burundi)"},
    {"fr_CM", "French (Cameroon)"},
    {"fr_CA", "French (Canada)"},
    {"fr_CF", "French (Central African Republic)"},
    {"fr_TD", "French (Chad)"},
    {"fr_KM", "French (Comoros)"},
    {"fr_CG", "French (Congo - Brazzaville)"},
    {"fr_CD", "French (Congo - Kinshasa)"},
    {"fr_CI", "French (CГґte dвЂ™Ivoire)"},
    {"fr_DJ", "French (Djibouti)"},
    {"fr_GQ", "French (Equatorial Guinea)"},
    {"fr_FR", "French (France)"},
    {"fr_GA", "French (Gabon)"},
    {"fr_GP", "French (Guadeloupe)"},
    {"fr_GN", "French (Guinea)"},
    {"fr_LU", "French (Luxembourg)"},
    {"fr_MG", "French (Madagascar)"},
    {"fr_ML", "French (Mali)"},
    {"fr_MQ", "French (Martinique)"},
    {"fr_MC", "French (Monaco)"},
    {"fr_NE", "French (Niger)"},
    {"fr_RW", "French (Rwanda)"},
    {"fr_RE", "French (RГ©union)"},
    {"fr_BL", "French (Saint BarthГ©lemy)"},
    {"fr_MF", "French (Saint Martin)"},
    {"fr_SN", "French (Senegal)"},
    {"fr_CH", "French (Switzerland)"},
    {"fr_TG", "French (Togo)"},
    {"fr", "French"},
    {"ff_SN", "Fulah (Senegal)"},
    {"ff", "Fulah"},
    {"gl_ES", "Galician (Spain)"},
    {"gl", "Galician"},
    {"lg_UG", "Ganda (Uganda)"},
    {"lg", "Ganda"},
    {"ka_GE", "Georgian (Georgia)"},
    {"ka", "Georgian"},
    {"de_AT", "German (Austria)"},
    {"de_BE", "German (Belgium)"},
    {"de_DE", "German (Germany)"},
    {"de_LI", "German (Liechtenstein)"},
    {"de_LU", "German (Luxembourg)"},
    {"de_CH", "German (Switzerland)"},
    {"de", "German"},
    {"el_CY", "Greek (Cyprus)"},
    {"el_GR", "Greek (Greece)"},
    {"el", "Greek"},
    {"gu_IN", "Gujarati (India)"},
    {"gu", "Gujarati"},
    {"guz_KE", "Gusii (Kenya)"},
    {"guz", "Gusii"},
    {"ha_Latn", "Hausa (Latin)"},
    {"ha_Latn_GH", "Hausa (Latin, Ghana)"},
    {"ha_Latn_NE", "Hausa (Latin, Niger)"},
    {"ha_Latn_NG", "Hausa (Latin, Nigeria)"},
    {"ha", "Hausa"},
    {"haw_US", "Hawaiian (United States)"},
    {"haw", "Hawaiian"},
    {"he_IL", "Hebrew (Israel)"},
    {"he", "Hebrew"},
    {"hi_IN", "Hindi (India)"},
    {"hi", "Hindi"},
    {"hu_HU", "Hungarian (Hungary)"},
    {"hu", "Hungarian"},
    {"is_IS", "Icelandic (Iceland)"},
    {"is", "Icelandic"},
    {"ig_NG", "Igbo (Nigeria)"},
    {"ig", "Igbo"},
    {"id_ID", "Indonesian (Indonesia)"},
    {"id", "Indonesian"},
    {"ga_IE", "Irish (Ireland)"},
    {"ga", "Irish"},
    {"it_IT", "Italian (Italy)"},
    {"it_CH", "Italian (Switzerland)"},
    {"it", "Italian"},
    {"ja_JP", "Japanese (Japan)"},
    {"ja", "Japanese"},
    {"kea_CV", "Kabuverdianu (Cape Verde)"},
    {"kea", "Kabuverdianu"},
    {"kab_DZ", "Kabyle (Algeria)"},
    {"kab", "Kabyle"},
    {"kl_GL", "Kalaallisut (Greenland)"},
    {"kl", "Kalaallisut"},
    {"kln_KE", "Kalenjin (Kenya)"},
    {"kln", "Kalenjin"},
    {"kam_KE", "Kamba (Kenya)"},
    {"kam", "Kamba"},
    {"kn_IN", "Kannada (India)"},
    {"kn", "Kannada"},
    {"kk_Cyrl", "Kazakh (Cyrillic)"},
    {"kk_Cyrl_KZ", "Kazakh (Cyrillic, Kazakhstan)"},
    {"kk", "Kazakh"},
    {"km_KH", "Khmer (Cambodia)"},
    {"km", "Khmer"},
    {"ki_KE", "Kikuyu (Kenya)"},
    {"ki", "Kikuyu"},
    {"rw_RW", "Kinyarwanda (Rwanda)"},
    {"rw", "Kinyarwanda"},
    {"kok_IN", "Konkani (India)"},
    {"kok", "Konkani"},
    {"ko_KR", "Korean (South Korea)"},
    {"ko", "Korean"},
    {"khq_ML", "Koyra Chiini (Mali)"},
    {"khq", "Koyra Chiini"},
    {"ses_ML", "Koyraboro Senni (Mali)"},
    {"ses", "Koyraboro Senni"},
    {"lag_TZ", "Langi (Tanzania)"},
    {"lag", "Langi"},
    {"lv_LV", "Latvian (Latvia)"},
    {"lv", "Latvian"},
    {"lt_LT", "Lithuanian (Lithuania)"},
    {"lt", "Lithuanian"},
    {"luo_KE", "Luo (Kenya)"},
    {"luo", "Luo"},
    {"luy_KE", "Luyia (Kenya)"},
    {"luy", "Luyia"},
    {"mk_MK", "Macedonian (Macedonia)"},
    {"mk", "Macedonian"},
    {"jmc_TZ", "Machame (Tanzania)"},
    {"jmc", "Machame"},
    {"kde_TZ", "Makonde (Tanzania)"},
    {"kde", "Makonde"},
    {"mg_MG", "Malagasy (Madagascar)"},
    {"mg", "Malagasy"},
    {"ms_BN", "Malay (Brunei)"},
    {"ms_MY", "Malay (Malaysia)"},
    {"ms", "Malay"},
    {"ml_IN", "Malayalam (India)"},
    {"ml", "Malayalam"},
    {"mt_MT", "Maltese (Malta)"},
    {"mt", "Maltese"},
    {"gv_GB", "Manx (United Kingdom)"},
    {"gv", "Manx"},
    {"mr_IN", "Marathi (India)"},
    {"mr", "Marathi"},
    {"mas_KE", "Masai (Kenya)"},
    {"mas_TZ", "Masai (Tanzania)"},
    {"mas", "Masai"},
    {"mer_KE", "Meru (Kenya)"},
    {"mer", "Meru"},
    {"mfe_MU", "Morisyen (Mauritius)"},
    {"mfe", "Morisyen"},
    {"naq_NA", "Nama (Namibia)"},
    {"naq", "Nama"},
    {"ne_IN", "Nepali (India)"},
    {"ne_NP", "Nepali (Nepal)"},
    {"ne", "Nepali"},
    {"nd_ZW", "North Ndebele (Zimbabwe)"},
    {"nd", "North Ndebele"},
    {"nb_NO", "Norwegian BokmГҐl (Norway)"},
    {"nb", "Norwegian BokmГҐl"},
    {"nn_NO", "Norwegian Nynorsk (Norway)"},
    {"nn", "Norwegian Nynorsk"},
    {"nyn_UG", "Nyankole (Uganda)"},
    {"nyn", "Nyankole"},
    {"or_IN", "Oriya (India)"},
    {"or", "Oriya"},
    {"om_ET", "Oromo (Ethiopia)"},
    {"om_KE", "Oromo (Kenya)"},
    {"om", "Oromo"},
    {"ps_AF", "Pashto (Afghanistan)"},
    {"ps", "Pashto"},
    {"fa_AF", "Persian (Afghanistan)"},
    {"fa_IR", "Persian (Iran)"},
    {"fa", "Persian"},
    {"pl_PL", "Polish (Poland)"},
    {"pl", "Polish"},
    {"pt_BR", "Portuguese (Brazil)"},
    {"pt_GW", "Portuguese (Guinea-Bissau)"},
    {"pt_MZ", "Portuguese (Mozambique)"},
    {"pt_PT", "Portuguese (Portugal)"},
    {"pt", "Portuguese"},
    {"pa_Arab", "Punjabi (Arabic)"},
    {"pa_Arab_PK", "Punjabi (Arabic, Pakistan)"},
    {"pa_Guru", "Punjabi (Gurmukhi)"},
    {"pa_Guru_IN", "Punjabi (Gurmukhi, India)"},
    {"pa", "Punjabi"},
    {"ro_MD", "Romanian (Moldova)"},
    {"ro_RO", "Romanian (Romania)"},
    {"ro", "Romanian"},
    {"rm_CH", "Romansh (Switzerland)"},
    {"rm", "Romansh"},
    {"rof_TZ", "Rombo (Tanzania)"},
    {"rof", "Rombo"},
    {"ru_MD", "Russian (Moldova)"},
    {"ru_RU", "Russian (Russia)"},
    {"ru_UA", "Russian (Ukraine)"},
    {"ru", "Russian"},
    {"rwk_TZ", "Rwa (Tanzania)"},
    {"rwk", "Rwa"},
    {"saq_KE", "Samburu (Kenya)"},
    {"saq", "Samburu"},
    {"sg_CF", "Sango (Central African Republic)"},
    {"sg", "Sango"},
    {"seh_MZ", "Sena (Mozambique)"},
    {"seh", "Sena"},
    {"sr_Cyrl", "Serbian (Cyrillic)"},
    {"sr_Cyrl_BA", "Serbian (Cyrillic, Bosnia and Herzegovina)"},
    {"sr_Cyrl_ME", "Serbian (Cyrillic, Montenegro)"},
    {"sr_Cyrl_RS", "Serbian (Cyrillic, Serbia)"},
    {"sr_Latn", "Serbian (Latin)"},
    {"sr_Latn_BA", "Serbian (Latin, Bosnia and Herzegovina)"},
    {"sr_Latn_ME", "Serbian (Latin, Montenegro)"},
    {"sr_Latn_RS", "Serbian (Latin, Serbia)"},
    {"sr", "Serbian"},
    {"sn_ZW", "Shona (Zimbabwe)"},
    {"sn", "Shona"},
    {"ii_CN", "Sichuan Yi (China)"},
    {"ii", "Sichuan Yi"},
    {"si_LK", "Sinhala (Sri Lanka)"},
    {"si", "Sinhala"},
    {"sk_SK", "Slovak (Slovakia)"},
    {"sk", "Slovak"},
    {"sl_SI", "Slovenian (Slovenia)"},
    {"sl", "Slovenian"},
    {"xog_UG", "Soga (Uganda)"},
    {"xog", "Soga"},
    {"so_DJ", "Somali (Djibouti)"},
    {"so_ET", "Somali (Ethiopia)"},
    {"so_KE", "Somali (Kenya)"},
    {"so_SO", "Somali (Somalia)"},
    {"so", "Somali"},
    {"es_AR", "Spanish (Argentina)"},
    {"es_BO", "Spanish (Bolivia)"},
    {"es_CL", "Spanish (Chile)"},
    {"es_CO", "Spanish (Colombia)"},
    {"es_CR", "Spanish (Costa Rica)"},
    {"es_DO", "Spanish (Dominican Republic)"},
    {"es_EC", "Spanish (Ecuador)"},
    {"es_SV", "Spanish (El Salvador)"},
    {"es_GQ", "Spanish (Equatorial Guinea)"},
    {"es_GT", "Spanish (Guatemala)"},
    {"es_HN", "Spanish (Honduras)"},
    {"es_419", "Spanish (Latin America)"},
    {"es_MX", "Spanish (Mexico)"},
    {"es_NI", "Spanish (Nicaragua)"},
    {"es_PA", "Spanish (Panama)"},
    {"es_PY", "Spanish (Paraguay)"},
    {"es_PE", "Spanish (Peru)"},
    {"es_PR", "Spanish (Puerto Rico)"},
    {"es_ES", "Spanish (Spain)"},
    {"es_US", "Spanish (United States)"},
    {"es_UY", "Spanish (Uruguay)"},
    {"es_VE", "Spanish (Venezuela)"},
    {"es", "Spanish"},
    {"sw_KE", "Swahili (Kenya)"},
    {"sw_TZ", "Swahili (Tanzania)"},
    {"sw", "Swahili"},
    {"sv_FI", "Swedish (Finland)"},
    {"sv_SE", "Swedish (Sweden)"},
    {"sv", "Swedish"},
    {"gsw_CH", "Swiss German (Switzerland)"},
    {"gsw", "Swiss German"},
    {"shi_Latn", "Tachelhit (Latin)"},
    {"shi_Latn_MA", "Tachelhit (Latin, Morocco)"},
    {"shi_Tfng", "Tachelhit (Tifinagh)"},
    {"shi_Tfng_MA", "Tachelhit (Tifinagh, Morocco)"},
    {"shi", "Tachelhit"},
    {"dav_KE", "Taita (Kenya)"},
    {"dav", "Taita"},
    {"ta_IN", "Tamil (India)"},
    {"ta_LK", "Tamil (Sri Lanka)"},
    {"ta", "Tamil"},
    {"te_IN", "Telugu (India)"},
    {"te", "Telugu"},
    {"teo_KE", "Teso (Kenya)"},
    {"teo_UG", "Teso (Uganda)"},
    {"teo", "Teso"},
    {"th_TH", "Thai (Thailand)"},
    {"th", "Thai"},
    {"bo_CN", "Tibetan (China)"},
    {"bo_IN", "Tibetan (India)"},
    {"bo", "Tibetan"},
    {"ti_ER", "Tigrinya (Eritrea)"},
    {"ti_ET", "Tigrinya (Ethiopia)"},
    {"ti", "Tigrinya"},
    {"to_TO", "Tonga (Tonga)"},
    {"to", "Tonga"},
    {"tr_TR", "Turkish (Turkey)"},
    {"tr", "Turkish"},
    {"uk_UA", "Ukrainian (Ukraine)"},
    {"uk", "Ukrainian"},
    {"ur_IN", "Urdu (India)"},
    {"ur_PK", "Urdu (Pakistan)"},
    {"ur", "Urdu"},
    {"uz_Arab", "Uzbek (Arabic)"},
    {"uz_Arab_AF", "Uzbek (Arabic, Afghanistan)"},
    {"uz_Cyrl", "Uzbek (Cyrillic)"},
    {"uz_Cyrl_UZ", "Uzbek (Cyrillic, Uzbekistan)"},
    {"uz_Latn", "Uzbek (Latin)"},
    {"uz_Latn_UZ", "Uzbek (Latin, Uzbekistan)"},
    {"uz", "Uzbek"},
    {"vi_VN", "Vietnamese (Vietnam)"},
    {"vi", "Vietnamese"},
    {"vun_TZ", "Vunjo (Tanzania)"},
    {"vun", "Vunjo"},
    {"cy_GB", "Welsh (United Kingdom)"},
    {"cy", "Welsh"},
    {"yo_NG", "Yoruba (Nigeria)"},
    {"yo", "Yoruba"},
    {"zu_ZA", "Zulu (South Africa)"},
    {"zu", "Zulu"},
    };
// TODO: rewrite this shit
BYTE hex_digit(TCHAR f)
{
    BYTE p;
    if (f >= _T('0') && f <= _T('9')) {
        p = static_cast<BYTE>(f - _T('0'));
    } else {
        p = static_cast<BYTE>(f - _T('a') + 10);
    }
    return p;
}

int hexstr2int(LPTSTR hex)
{
    int len = lstrlen(hex);
    int  step = 1;

    BYTE b[4];
    if (len > 8)
        return 0;
    for (int i = 0; i < len; i += 2) {
        //ATLASSERT(i / 2 <= 3);
        b[i / 2] = hex_digit(hex[i]) * 16 + hex_digit(hex[i + 1]);
        step *= 16;
    }
    return *(DWORD*)&b;
}

int myhash(PBYTE key, int len)
{
    int hash = 222;
    for (int i = 0; i < len; ++i) {
        hash = (hash ^ key[i]) + ((hash << 26) + (hash >> 6));
    }

    return hash;
}

LPTSTR fgetline(LPTSTR buf, int num, FILE *f)
{
    LPTSTR Result;
    LPTSTR cur;
    Result = _fgetts(buf, num, f);
    int n = lstrlen(buf) - 1;

    for (cur = buf + n; cur >= buf; cur--)
    {
        if (*cur == 13 || *cur == 10 || *cur == _T('\n'))
            *cur = 0;
        else break;
    }
    return Result;
}

size_t GetFolderFileList(std::vector<CString>& list, CString folder, CString mask)
{
    WIN32_FIND_DATA wfd;
    ZeroMemory(&wfd, sizeof(wfd));
    HANDLE findfile = nullptr;
    TCHAR szNameBuffer[MAX_PATH];

    for (;; )
    {
        if (!findfile)
        {
            findfile = FindFirstFile(folder + _T("\\") + mask, &wfd);
            if (!findfile)
                break;
            ;
        }
        else
        {
            if (!FindNextFile(findfile, &wfd))
                break;
        }
        if (lstrlen(wfd.cFileName) < 1)
            break;
        lstrcpyn(szNameBuffer, wfd.cFileName, 254);
        list.push_back(szNameBuffer);
    }
    // return TRUE;

    // error:
    if (findfile)
        FindClose(findfile);
    return list.size();
    // return FALSE;
}

bool ReadUtf8TextFile(const CString& fileName, std::string& data)
{
    FILE *stream = _wfopen(fileName, L"rb");
    if (!stream) {
        return false;
    }
    fseek(stream, 0L, SEEK_END);
    size_t size = ftell(stream);
    rewind(stream);

    unsigned char buf[3] = { 0,0,0 };
    size_t bytesRead = fread(buf, 1, 3, stream);

    if (bytesRead == 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF) // UTF8 Byte Order Mark (BOM)
    {
        size -= 3;
    }
    else if (bytesRead >= 2 && buf[0] == 0xFF && buf[1] == 0xFE) {
        // UTF-16LE encoding
        // not supported
        fclose(stream);
        return false;
    }
    else {
        // no BOM was found; seeking backward
        fseek(stream, 0L, SEEK_SET);
    }
    try {
        data.resize(size);
    }
    catch (std::exception& ) {
        //LOG(ERROR) << ex.what();
        fclose(stream);
        return false;
    }

    size_t bytesRead2 = fread(&data[0], 1, size, stream);
    if (bytesRead2 == size) {
        fclose(stream);
        return true;
    }

    fclose(stream);
    return false;
}

std::wstring StringToWideString(const std::string &str, UINT codePage)
{
    std::wstring ws;
    int n = MultiByteToWideChar(codePage, 0, str.c_str(), static_cast<int>(str.size() + 1), /*dst*/NULL, 0);
    if (n) {
        ws.reserve(n);
        ws.resize(n - 1);
        if (MultiByteToWideChar(codePage, 0, str.c_str(), static_cast<int>(str.size() + 1), /*dst*/&ws[0], n) == 0)
            ws.clear();
    }
    return ws;
}

std::string WideStringToString(const std::wstring& ws, UINT codePage)
{
    // prior to C++11 std::string and std::wstring were not guaranteed to have their memory be contiguous,
    // although all real-world implementations make them contiguous
    std::string str;
    int srcLen = static_cast<int>(ws.size());
    int n = WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen + 1, NULL, 0, /*defchr*/0, NULL);
    if (n) {
        str.reserve(n);
        str.resize(n - 1);
        if (WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen + 1, &str[0], n, /*defchr*/0, NULL) == 0)
            str.clear();
    }
    return str;
}

}

CLang::CLang()
{
    locale_ = "en_US";
    language_ = "en";
    isRTL_ = false;
}
void CLang::SetDirectory(LPCTSTR Directory)
{
    m_Directory = Directory;
}

bool CLang::LoadLanguage(LPCTSTR Lang)
{
    StringList.clear();
    if (!Lang ) {
        return false;
    }

    CString Filename = CString(m_Directory) + Lang + _T(".lng");

    std::string fileContents;

    if (Lang == CString("English")) { // English is a built-in language
        currentLanguageFile_ = Filename;
        return true;
    }

    if (!ReadUtf8TextFile(Filename, fileContents)) {
       
        return false;
    }
    std::wstring fileContentsW = StringToWideString(fileContents, CP_UTF8);

    std::wstringstream ss(fileContentsW);
    std::wstring line;

    CString Buffer;
    CString Name;
    CString Text;

    while (std::getline(ss, line)) {
        
        Buffer.Empty();
        Name.Empty();
        Text.Empty();
        if (!line.empty() && line[line.length()-1]==L'\r') {
            line.pop_back(); // remove last character
        }
        Buffer = line.c_str();

        if (*Buffer == _T('#'))
            continue;

        int pos = Buffer.Find('=');

        if (pos!=-1) {
            Name = Buffer.Left(pos);
            Text = Buffer.Mid(pos + 1);
        }

        Name.Trim();

        CString RepText = Text;
        RepText.Replace(_T("\\n"), _T("\r\n"));

        if (!RepText.IsEmpty() && RepText[0] == _T(' ')) {
            RepText = RepText.Mid(1);
        }

        int NameLen = Name.GetLength();
        int TextLen = RepText.GetLength();

        if (!NameLen || !TextLen)
            continue;

        TCHAR* pName = new TCHAR[NameLen + 1];
        TCHAR* pText = new TCHAR[TextLen + 1];

        lstrcpy(pName, Name);
        lstrcpy(pText, RepText);

        if ( Name == CString("language") ) {
            locale_ = pText;
            language_ = locale_.Left(locale_.Find('_'));
            delete[] pName;
            delete[] pText;
            continue;
        } else if (Name == CString(_T("RTL"))) {
            CString lowerText = CString(pText).MakeLower();
            if (lowerText == "yes" || lowerText == "1" || lowerText == "true") {
                isRTL_ = true;
            }
            delete[] pName;
            delete[] pText;
            continue;
        }

        TranslateListItem tli = {nullptr, nullptr};
        tli.Name = pName;
        tli.Text = pText;
        int hash = hexstr2int(pName);
        StringList[hash] = tli;
    }

    m_sLang = Lang;
    currentLanguageFile_ = Filename;
    return true;
}

LPCTSTR CLang::GetString(LPCTSTR Name) const {
    int hash = myhash((PBYTE)Name, lstrlen(Name) * sizeof(TCHAR));
    auto it = StringList.find(hash);
    if (it != StringList.end()) {
        return it->second.Text;
    }

    // return _T("$NO_SUCH_STRING");
    return Name;
}

CString CLang::GetLanguageName() const
{
    return m_sLang;
}

CString CLang::getLanguage() const
{
    return language_;
}

CString CLang::getLocale() const
{
    return locale_;
}
#ifndef IU_SHELLEXT
std::string CLang::getCurrentLanguage() {
    return W2U(m_sLang);
}

std::string CLang::getCurrentLocale() {
    return W2U(locale_);
}

std::string CLang::translate(const char* str) {
    std::wstring wideStr = IuCoreUtils::Utf8ToWstring(str);
    return IuCoreUtils::WstringToUtf8(GetString(wideStr.c_str()));
}

const wchar_t* CLang::translateW(const wchar_t* str)  {
    return GetString(str);
}
#endif

CString CLang::getLanguageFileNameForLocale(const CString& locale)
{
    std::vector<CString> list;
    GetFolderFileList(list, m_Directory, _T("*.lng"));
    CString foundName;

    for(const auto& fileName: list )
    {
        FILE* f = _tfopen(m_Directory + fileName, _T("rb"));
        if (!f) {
            continue;
        }
        //fseek(f, 2, SEEK_SET); // skipping BOM
        TCHAR buffer[1024];

        while (!feof(f))
        {
            memset(buffer, 0, sizeof(buffer));
            fgetline(buffer, 1024, f);
            CString buf = buffer;
            if (buf.GetLength() && buf[0] == _T('#')) {
                continue;
            }
            
            int equalSignPos = buf.Find(L'=');
            CString key = buf.Left(equalSignPos);
            key.TrimRight(L" ");
            CString value = buf.Right(buf.GetLength() - equalSignPos-1);
            value.TrimLeft(L" ");
            if ( key == "language" ) {
                if ( value == locale ) {
                    foundName = fileName.Left(fileName.ReverseFind('.'));
                    fclose(f);
                    return foundName;
                }
                CString lang = value.Left(value.Find('_'));
                if (  lang == locale ) {
                    foundName = fileName.Left(fileName.ReverseFind('.'));
                    break;
                }
            }
        }
        fclose(f);
    }
    return foundName;
}

bool CLang::isRTL() const {
    return isRTL_;
}

CLang::~CLang()
{
    for (auto& it : StringList) {
        delete[] it.second.Name;
        delete[] it.second.Text;
    }
}

CString CLang::getCurrentLanguageFile() const {
    return currentLanguageFile_;
}