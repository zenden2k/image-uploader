#include <cstring>

#include <curl/curl.h>
#include <gtest/gtest.h>

#include "Tests/TestHelpers.h"

class NetworkClientTest : public ::testing::Test {
public:
    NetworkClientTest() {
    }

    static bool arrayContains(const char* const* protocols, const char* str) {
        size_t i = 0;
        const char* protocol;
        while ((protocol = protocols[i++]) != nullptr) {
            if (!strcmp(protocol, str)) {
                return true;
            }
            //protocol = data->protocols[i];
        }
        return false;
    }
};

TEST_F(NetworkClientTest, curlTest) {
    curl_version_info_data * data = curl_version_info(CURLVERSION_NOW);
    ASSERT_TRUE(data->protocols != nullptr);  
    ASSERT_TRUE(data->feature_names != nullptr);  
    EXPECT_TRUE(arrayContains(data->protocols, "sftp"));
    EXPECT_TRUE(arrayContains(data->protocols, "ftp"));
    EXPECT_TRUE(arrayContains(data->protocols, "http"));
    EXPECT_TRUE(arrayContains(data->protocols, "https"));
#ifdef _WIN32
    EXPECT_TRUE(arrayContains(data->feature_names, "Unicode"));
#endif
    EXPECT_TRUE(arrayContains(data->feature_names, "HTTPS-proxy"));
    EXPECT_TRUE(data->ssl_version != nullptr);
}