#include <cstring>

#include <curl/curl.h>
#include <gtest/gtest.h>


class CurlTest : public ::testing::Test {
public:
    static bool arrayContains(const char* const* protocols, const char* str) {
        size_t i = 0;
        const char* protocol;
        while ((protocol = protocols[i++]) != nullptr) {
            if (!strcmp(protocol, str)) {
                return true;
            }
        }
        return false;
    }
};

TEST_F(CurlTest, CurlTest) {
    curl_version_info_data * data = curl_version_info(CURLVERSION_NOW);
    ASSERT_TRUE(data->protocols != nullptr);  
    ASSERT_TRUE(data->feature_names != nullptr);  
    EXPECT_TRUE(arrayContains(data->protocols, "sftp"));
    EXPECT_TRUE(arrayContains(data->protocols, "ftp"));
    EXPECT_TRUE(arrayContains(data->protocols, "http"));
    EXPECT_TRUE(arrayContains(data->protocols, "https"));

    EXPECT_NE(data->features & CURL_VERSION_SSL, 0);
#ifdef _WIN32
    EXPECT_NE(data->features & CURL_VERSION_UNICODE, 0);
#endif
    EXPECT_NE(data->features & CURL_VERSION_HTTPS_PROXY, 0);
    EXPECT_TRUE(data->ssl_version != nullptr);
}

