#include <gtest/gtest.h>
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/TextUtils.h"
#include <boost/format.hpp>
#include <fstream>

using namespace IuTextUtils;

// Tests from:
// https://github.com/pagespeed/mod_pagespeed/blob/0e6dedb27d68353797a983ae36b20dd7b40ede47/pagespeed/kernel/html/html_keywords_test.cc

class TextUtilsTest : public ::testing::Test {
protected:

    void SetUp() override {
    };

    void TestEscape(const std::string& symbolic_code, char value, const char* expected) {
        std::string symbolic_escaped = "&"+ symbolic_code+";";
        char numeric_escaped[20];
        sprintf(numeric_escaped, "&#%02d;", static_cast<unsigned char>(value));
        EXPECT_EQ(expected, DecodeHtmlEntities(numeric_escaped));
        EXPECT_EQ(expected, DecodeHtmlEntities(symbolic_escaped));
    }

    // Validates that the provided text is not altered by escaping it.
    void Unchanged(const std::string& text) {
        //EXPECT_STREQ(text, HtmlKeywords::Escape(text, &buf));
        EXPECT_EQ(text, DecodeHtmlEntities(text));
    }

    void TearDown() override {

    };
};

TEST_F(TextUtilsTest, DecodeHtmlEntities)
{
    std::string result = DecodeHtmlEntities("Foo &#xA9; bar &#x1D306; baz &#x2603; qux");
    EXPECT_EQ(result, "\x46\x6F\x6F\x20\xC2\xA9\x20\x62\x61\x72\x20\xF0\x9D\x8C\x86\x20\x62\x61\x7A\x20\xE2\x98\x83\x20\x71\x75\x78");
    static const char kListView[] =
        "http://list.taobao.com/market/baby.htm?spm=1.151829.71436.25&"
        "cat=50032645&sort=_bid&spercent=95&isprepay=1&user_type=0&gobaby=1&"
        "random=false&lstyle=imgw&as=1&viewIndex=1&yp4p_page=0&commend=all&"
        "atype=b&style=grid&olu=yes&isnew=2&mSelect=false&#ListView";
    EXPECT_EQ(kListView, DecodeHtmlEntities(kListView));
    EXPECT_EQ("&&", DecodeHtmlEntities("&&"));
    EXPECT_EQ("&&", DecodeHtmlEntities("&amp;&amp;"));
    //EXPECT_EQ("&", DecodeHtmlEntities("&amp"));
    //EXPECT_EQ("&&", DecodeHtmlEntities("&amp&amp"));
    EXPECT_EQ("&ocircoooo", DecodeHtmlEntities("&ocircoooo"));
    EXPECT_EQ("&ocircoooo", DecodeHtmlEntities("&amp;ocircoooo"));

    Unchanged("a b");
    Unchanged("a\nb");
    Unchanged("a\rb");
    Unchanged("a\tb");
    Unchanged("a\fb");
}



TEST_F(TextUtilsTest, AllCodes)
{
    TestEscape("AElig", 0xc6, "\xc3\x86");
    TestEscape("Aacute", 0xc1, "\xc3\x81");
    TestEscape("Acirc", 0xc2, "\xc3\x82");
    TestEscape("Agrave", 0xc0, "\xc3\x80");
    TestEscape("Aring", 0xc5, "\xc3\x85");
    TestEscape("Atilde", 0xc3, "\xc3\x83");
    TestEscape("Auml", 0xc4, "\xc3\x84");
    TestEscape("Ccedil", 0xc7, "\xc3\x87");
    TestEscape("ETH", 0xd0, "\xc3\x90");
    TestEscape("Eacute", 0xc9, "\xc3\x89");
    TestEscape("Ecirc", 0xca, "\xc3\x8a");
    TestEscape("Egrave", 0xc8, "\xc3\x88");
    TestEscape("Euml", 0xcb, "\xc3\x8b");
    TestEscape("Iacute", 0xcd, "\xc3\x8d");
    TestEscape("Icirc", 0xce, "\xc3\x8e");
    TestEscape("Igrave", 0xcc, "\xc3\x8c");
    TestEscape("Iuml", 0xcf, "\xc3\x8f");
    TestEscape("Ntilde", 0xd1, "\xc3\x91");
    TestEscape("Oacute", 0xd3, "\xc3\x93");
    TestEscape("Ocirc", 0xd4, "\xc3\x94");
    TestEscape("Ograve", 0xd2, "\xc3\x92");
    TestEscape("Oslash", 0xd8, "\xc3\x98");
    TestEscape("Otilde", 0xd5, "\xc3\x95");
    TestEscape("Ouml", 0xd6, "\xc3\x96");
    TestEscape("THORN", 0xde, "\xc3\x9e");
    TestEscape("Uacute", 0xda, "\xc3\x9a");
    TestEscape("Ucirc", 0xdb, "\xc3\x9b");
    TestEscape("Ugrave", 0xd9, "\xc3\x99");
    TestEscape("Uuml", 0xdc, "\xc3\x9c");
    TestEscape("Yacute", 0xdd, "\xc3\x9d");
    TestEscape("aacute", 0xe1, "\xc3\xa1");
    TestEscape("acirc", 0xe2, "\xc3\xa2");
    TestEscape("acute", 0xb4, "\xc2\xb4");
    TestEscape("aelig", 0xe6, "\xc3\xa6");
    TestEscape("agrave", 0xe0, "\xc3\xa0");
    TestEscape("amp", 0x26, "\x26");
    TestEscape("aring", 0xe5, "\xc3\xa5");
    TestEscape("atilde", 0xe3, "\xc3\xa3");
    TestEscape("auml", 0xe4, "\xc3\xa4");
    TestEscape("brvbar", 0xa6, "\xc2\xa6");
    TestEscape("ccedil", 0xe7, "\xc3\xa7");
    TestEscape("cedil", 0xb8, "\xc2\xb8");
    TestEscape("cent", 0xa2, "\xc2\xa2");
    TestEscape("copy", 0xa9, "\xc2\xa9");
    TestEscape("curren", 0xa4, "\xc2\xa4");
    TestEscape("deg", 0xb0, "\xc2\xb0");
    TestEscape("divide", 0xf7, "\xc3\xb7");
    TestEscape("eacute", 0xe9, "\xc3\xa9");
    TestEscape("ecirc", 0xea, "\xc3\xaa");
    TestEscape("egrave", 0xe8, "\xc3\xa8");
    TestEscape("eth", 0xf0, "\xc3\xb0");
    TestEscape("euml", 0xeb, "\xc3\xab");
    TestEscape("frac12", 0xbd, "\xc2\xbd");
    TestEscape("frac14", 0xbc, "\xc2\xbc");
    TestEscape("frac34", 0xbe, "\xc2\xbe");
    TestEscape("gt", 0x3e, "\x3e");
    TestEscape("iacute", 0xed, "\xc3\xad");
    TestEscape("icirc", 0xee, "\xc3\xae");
    TestEscape("iexcl", 0xa1, "\xc2\xa1");
    TestEscape("igrave", 0xec, "\xc3\xac");
    TestEscape("iquest", 0xbf, "\xc2\xbf");
    TestEscape("iuml", 0xef, "\xc3\xaf");
    TestEscape("laquo", 0xab, "\xc2\xab");
    TestEscape("lt", 0x3c, "\x3c");
    TestEscape("macr", 0xaf, "\xc2\xaf");
    TestEscape("micro", 0xb5, "\xc2\xb5");
    TestEscape("middot", 0xb7, "\xc2\xb7");
    TestEscape("nbsp", 0xa0, "\xc2\xa0");
    TestEscape("not", 0xac, "\xc2\xac");
    TestEscape("ntilde", 0xf1, "\xc3\xb1");
    TestEscape("oacute", 0xf3, "\xc3\xb3");
    TestEscape("ocirc", 0xf4, "\xc3\xb4");
    TestEscape("ograve", 0xf2, "\xc3\xb2");
    TestEscape("ordf", 0xaa, "\xc2\xaa");
    TestEscape("ordm", 0xba, "\xc2\xba");
    TestEscape("oslash", 0xf8, "\xc3\xb8");
    TestEscape("otilde", 0xf5, "\xc3\xb5");
    TestEscape("ouml", 0xf6, "\xc3\xb6");
    TestEscape("para", 0xb6, "\xc2\xb6");
    TestEscape("plusmn", 0xb1, "\xc2\xb1");
    TestEscape("pound", 0xa3, "\xc2\xa3");
    TestEscape("quot", 0x22, "\x22");
    TestEscape("raquo", 0xbb, "\xc2\xbb");
    TestEscape("reg", 0xae, "\xc2\xae");
    TestEscape("sect", 0xa7, "\xc2\xa7");
    TestEscape("shy", 0xad, "\xc2\xad");
    TestEscape("sup1", 0xb9, "\xc2\xb9");
    TestEscape("sup2", 0xb2, "\xc2\xb2");
    TestEscape("sup3", 0xb3, "\xc2\xb3");
    TestEscape("szlig", 0xdf, "\xc3\x9f");
    TestEscape("thorn", 0xfe, "\xc3\xbe");
    TestEscape("times", 0xd7, "\xc3\x97");
    TestEscape("uacute", 0xfa, "\xc3\xba");
    TestEscape("ucirc", 0xfb, "\xc3\xbb");
    TestEscape("ugrave", 0xf9, "\xc3\xb9");
    TestEscape("uml", 0xa8, "\xc2\xa8");
    TestEscape("uuml", 0xfc, "\xc3\xbc");
    TestEscape("yacute", 0xfd, "\xc3\xbd");
    TestEscape("yen", 0xa5, "\xc2\xa5");
    TestEscape("yuml", 0xff, "\xc3\xbf");
}