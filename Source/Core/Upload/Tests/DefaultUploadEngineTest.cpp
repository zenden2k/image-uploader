#include <gtest/gtest.h>

#include "Core/Upload/DefaultUploadEngine.h"
#include "Core/Upload/ServerSync.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/UrlShorteningTask.h"
#include "Core/Network/Tests/NetworkClientMock.h"
#include "Tests/TestHelpers.h"

using namespace ::testing;

class DefaultUploadEngineTest : public ::testing::Test {
public:
    DefaultUploadEngineTest() : constSizeFileName(TestHelpers::resolvePath("file_with_const_size.png")) {

    }
protected:
    const std::string constSizeFileName;
};

TEST_F(DefaultUploadEngineTest, doUpload)
{
    using ::testing::_;
    // NiceMock is used to ignore uninterested calls
    NiceMock<MockINetworkClient> networkClient;
    ServerSync sync;
    CDefaultUploadEngine engine(&sync);
    engine.setNetworkClient(&networkClient);

    // Prepare CUploadEngineData instance
    CUploadEngineData ued;
    ued.Name = "test server";
    ued.TypeMask = CUploadEngineData::TypeImageServer;
    
    UploadAction action1;
    action1.Index = 0;
    action1.Type = "get";
    //action1.Referer = "http://referer.com/";
    action1.Url = "https://example.com";
    ued.Actions.push_back(action1);

    UploadAction action2;
    action2.Index = 1;
    action2.Type = "upload";
    action2.Url = "https://example.com/upload";
    action2.PostParams = "someparam=1;file=%filename%;thumbwidth=$(_THUMBWIDTH);thumbheight=$(_THUMBHEIGHT);"
        "rnd=$(_RAND16BITS);th=$(_THREADID);fname=$(_FILENAME);fnamenoext=$(_FILENAMEWITHOUTEXT);fileext=$(_FILEEXT)";
    ActionRegExp regExp;
    regExp.Pattern = "https://example\\.com/(.+)";
    regExp.Required = true;
    ActionVariable variable("fileId", 0);
    regExp.Variables.push_back(variable);
    action2.Regexes.push_back(regExp);
    ued.Actions.push_back(action2);
    ued.ImageUrlTemplate = "https://serv2.example.com/$(fileId)";
    ued.DownloadUrlTemplate = "https://serv3.example.com/view/$(fileId)";
    ued.ThumbUrlTemplate = "https://serv4.example.com/thumb/$(fileId)";
    engine.setUploadData(&ued);

    std::string displayName = IuCoreUtils::ExtractFileName(constSizeFileName);
    std::string fileNameNoExt = IuCoreUtils::ExtractFileNameNoExt(constSizeFileName);
    std::string fileExt = IuCoreUtils::ExtractFileExt(constSizeFileName);

    ON_CALL(networkClient, doGet(_)).WillByDefault(Return(true));
    ON_CALL(networkClient, doUploadMultipartData()).WillByDefault(Return(true));
    ON_CALL(networkClient, responseCode()).WillByDefault(Return(200));
    {
        testing::InSequence dummy;
        //EXPECT_CALL(networkClient, setReferer(action1.Referer));
        EXPECT_CALL(networkClient, doGet(action1.Url));
        EXPECT_CALL(networkClient, responseBody()).Times(AnyNumber());
        EXPECT_CALL(networkClient, addQueryParam("someparam", "1"));
        EXPECT_CALL(networkClient, addQueryParamFile("file", constSizeFileName, _, _));
        EXPECT_CALL(networkClient, addQueryParam("thumbwidth", "160"));
        EXPECT_CALL(networkClient, addQueryParam("thumbheight", "120"));
        EXPECT_CALL(networkClient, addQueryParam("rnd", StrNe("")));
        EXPECT_CALL(networkClient, addQueryParam("th", StrNe("")));
        EXPECT_CALL(networkClient, addQueryParam("fname", displayName));
        EXPECT_CALL(networkClient, addQueryParam("fnamenoext", fileNameNoExt));
        EXPECT_CALL(networkClient, addQueryParam("fileext", fileExt));

        EXPECT_CALL(networkClient, doUploadMultipartData());
        EXPECT_CALL(networkClient, responseBody()).WillOnce(Return("https://example.com/file_with_const_size.png"));
    }
    
    auto fileTask = std::make_shared<FileUploadTask>(constSizeFileName, displayName);
    ServerSettingsStruct serverSettings;
    engine.setServerSettings(&serverSettings);
    UploadParams uploadParams;
    uploadParams.thumbWidth = 160;
    uploadParams.thumbHeight = 120;
    int res = engine.doUpload(fileTask, uploadParams);
    EXPECT_EQ(1, res);
    EXPECT_EQ("https://serv2.example.com/file_with_const_size.png", uploadParams.getDirectUrl());
    EXPECT_EQ("https://serv4.example.com/thumb/file_with_const_size.png", uploadParams.getThumbUrl());
    EXPECT_EQ("https://serv3.example.com/view/file_with_const_size.png", uploadParams.getViewUrl());
}

TEST_F(DefaultUploadEngineTest, login)
{
    using ::testing::_;
    // NiceMock is used to ignore uninterested calls
    NiceMock<MockINetworkClient> networkClient;
    ServerSync sync;
    CDefaultUploadEngine engine(&sync);
    engine.setNetworkClient(&networkClient);

    // Prepare CUploadEngineData instance
    CUploadEngineData ued;
    ued.NeedAuthorization = CUploadEngineData::naAvailable;
    ued.Name = "test server2";
    ued.TypeMask = CUploadEngineData::TypeImageServer;

    UploadAction action1;
    action1.Index = 0;
    action1.Type = "login";
    action1.Url = "https://example.com/login";
    action1.Referer = "https://test.com/somepage";
    action1.PostParams = "password=$(_PASSWORD);login=$(_LOGIN);submit=1";

    ActionRegExp regExp;
    regExp.Pattern = "success";
    regExp.Required = true;
    action1.Regexes.push_back(regExp);

    ued.Actions.push_back(action1);

    ued.ImageUrlTemplate = "https://serv2.example.com/$(directUrl)";
    ued.DownloadUrlTemplate = "https://serv3.example.com/view/$(directUrl)";
    ued.ThumbUrlTemplate = "https://serv4.example.com/thumb/$(directUrl)";
    engine.setUploadData(&ued);

    ON_CALL(networkClient, doGet(_)).WillByDefault(Return(true));
    ON_CALL(networkClient, doPost(_)).WillByDefault(Return(true));
    ON_CALL(networkClient, responseCode()).WillByDefault(Return(200));
    {
        testing::InSequence dummy;
        EXPECT_CALL(networkClient, setReferer(action1.Referer));
        EXPECT_CALL(networkClient, addQueryParam("password", "qwerty"));
        EXPECT_CALL(networkClient, addQueryParam("login", "username"));
        EXPECT_CALL(networkClient, addQueryParam("submit", "1"));
        EXPECT_CALL(networkClient, doPost("")).WillOnce(Return(true));
        EXPECT_CALL(networkClient, responseBody()).WillOnce(Return("<result>success</result>"));
    }
    std::string displayName = IuCoreUtils::ExtractFileName(constSizeFileName);
    auto fileTask = std::make_shared<FileUploadTask>(constSizeFileName, displayName);
    ServerSettingsStruct serverSettings;
    serverSettings.authData.DoAuth = true;
    serverSettings.authData.Login = "username";
    serverSettings.authData.Password = "qwerty";

    engine.setServerSettings(&serverSettings);
    UploadParams uploadParams;
    int res = engine.doUpload(fileTask, uploadParams);
    EXPECT_EQ(1, res);
}

TEST_F(DefaultUploadEngineTest, shortenUrl)
{
    using ::testing::_;
    // NiceMock is used to ignore uninterested calls
    NiceMock<MockINetworkClient> networkClient;
    ServerSync sync;
    CDefaultUploadEngine engine(&sync);
    engine.setNetworkClient(&networkClient);

    // Prepare CUploadEngineData instance
    CUploadEngineData ued;
    ued.NeedAuthorization = CUploadEngineData::naNotAvailable;
    ued.Name = "test server3";
    ued.TypeMask = CUploadEngineData::TypeUrlShorteningServer;

    UploadAction action1;
    action1.Index = 0;
    action1.Type = "post";
    action1.Url = "https://example.com/shorten?url=$(_ORIGINALURL|urlencode)";
    action1.Referer = "https://test.com/somepage";
    action1.PostParams = "url_second_time=$(_ORIGINALURL);submit=1;";

    ActionRegExp regExp;
    regExp.Pattern = "<url>(.+)</url>";
    regExp.Required = true;
    ActionVariable variable("url", 0);
    regExp.Variables.push_back(variable);
    action1.Regexes.push_back(regExp);
    ued.Actions.push_back(action1);

    ued.ImageUrlTemplate = "$(url)";

    engine.setUploadData(&ued);
    std::string urlToShorten = "http://example.com/hello?someparam=1";
    ON_CALL(networkClient, doGet(_)).WillByDefault(Return(true));
    ON_CALL(networkClient, doPost(_)).WillByDefault(Return(true));
    ON_CALL(networkClient, responseCode()).WillByDefault(Return(200));
    ON_CALL(networkClient, responseBody()).WillByDefault(Return("<url>http://te.st/qwe1234</url>"));
    ON_CALL(networkClient, urlEncode(_)).WillByDefault(Return("http%3A%2F%2Fexample.com%2Fhello%3Fsomeparam%3D1"));

    EXPECT_CALL(networkClient, setReferer(action1.Referer));
    EXPECT_CALL(networkClient, setUrl("https://example.com/shorten?url=http%3A%2F%2Fexample.com%2Fhello%3Fsomeparam%3D1"));
    {
        testing::InSequence dummy;
        EXPECT_CALL(networkClient, addQueryParam("url_second_time", urlToShorten));
        EXPECT_CALL(networkClient, addQueryParam("submit", "1"));
        EXPECT_CALL(networkClient, doPost("")).WillOnce(Return(true)); 
    }
    std::string displayName = IuCoreUtils::ExtractFileName(constSizeFileName);
    auto fileTask = std::make_shared<UrlShorteningTask>(urlToShorten);
    ServerSettingsStruct serverSettings;
    engine.setServerSettings(&serverSettings);
    UploadParams uploadParams;
    int res = engine.doUpload(fileTask, uploadParams);
    EXPECT_EQ(1, res);
    EXPECT_EQ("http://te.st/qwe1234", uploadParams.getDirectUrl());
    EXPECT_EQ("", uploadParams.getThumbUrl());
    EXPECT_EQ("", uploadParams.getViewUrl());
}