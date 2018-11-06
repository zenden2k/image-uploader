#include <gtest/gtest.h>

#include "Core/UploadEngineList.h"
#include <Tests/TestHelpers.h>

class UploadEngineListTest : public ::testing::Test {
public:
    UploadEngineListTest() : fileName(TestHelpers::resolvePath("servers.xml")) {

    }
protected:
    const std::string fileName;
};

TEST_F(UploadEngineListTest, LenghtOfUtf8String)
{
    CUploadEngineList list;
    ServerSettingsMap settings;
    bool res = list.LoadFromFile(fileName, settings);
    // List is sorted alphabetically
    EXPECT_TRUE(res);
    EXPECT_EQ(3, list.count());
    CUploadEngineData* engineData = list.byIndex(2);
    ASSERT_TRUE(engineData != nullptr);
    EXPECT_EQ("radikal.ru", engineData->Name);
    EXPECT_EQ(10000000, engineData->MaxFileSize);
    EXPECT_EQ(false, engineData->Debug);
    EXPECT_EQ(1, engineData->NeedAuthorization);
    EXPECT_EQ(CUploadEngineData::TypeImageServer, engineData->TypeMask);
    EXPECT_EQ("radikal", engineData->PluginName);

    engineData = list.byName("fastpic.ru");
    ASSERT_TRUE(engineData != nullptr);
    EXPECT_EQ("fastpic.ru", engineData->Name);
    EXPECT_EQ(5000000, engineData->MaxFileSize);
    EXPECT_EQ(true, engineData->Debug);
    EXPECT_EQ(0, engineData->NeedAuthorization);
    EXPECT_EQ("", engineData->PluginName);
    EXPECT_EQ(CUploadEngineData::TypeImageServer, engineData->TypeMask);
    EXPECT_EQ("$(Image)", engineData->ImageUrlTemplate);
    EXPECT_EQ("$(Thumb)", engineData->ThumbUrlTemplate);
    EXPECT_EQ("$(DownloadUrl)", engineData->DownloadUrlTemplate);
    EXPECT_EQ(1, engineData->Actions.size());
    const UploadAction& action = engineData->Actions[0];
    EXPECT_EQ("http://fastpic.ru/upload?api=1", action.Url);
    EXPECT_EQ("method=file;file1=%filename%;check_thumb=size;uploading=1;orig_rotate=0;thumb_size=$(_THUMBWIDTH)", action.PostParams);
    EXPECT_EQ(2, action.Regexes.size());
    {
        const ActionRegExp& regexp = action.Regexes[0];
        EXPECT_EQ("<imagepath>(.*?)<\\/imagepath>([\\s\\S]*?)<thumbpath>(.*?)<\\/thumbpath>", regexp.Pattern);
        EXPECT_EQ("Image:0;Thumb:2", regexp.AssignVars);
        EXPECT_EQ(true, regexp.Required);
    }
    {
        const ActionRegExp& regexp = action.Regexes[1];
        EXPECT_EQ("<viewurl>(.*?)<\\/viewurl>", regexp.Pattern);
        EXPECT_EQ(false, regexp.Required);
        EXPECT_EQ("DownloadUrl:0;", regexp.AssignVars);
    }

    engineData = list.byName("8b.kz");
    ASSERT_TRUE(engineData != nullptr);
    EXPECT_EQ("8b.kz", engineData->Name);
    EXPECT_EQ(false, engineData->Debug);
    EXPECT_EQ(0, engineData->NeedAuthorization);
    EXPECT_EQ(CUploadEngineData::TypeUrlShorteningServer, engineData->TypeMask);
    EXPECT_EQ("http://8b.kz/$(surl)", engineData->ImageUrlTemplate);
    {
        EXPECT_EQ(1, engineData->Actions.size());
        const UploadAction& action = engineData->Actions[0];
        EXPECT_EQ("http://8b.kz/ajax/new_url", action.Url);
        EXPECT_EQ("post", action.Type);
        EXPECT_EQ("custom=;url=$(_ORIGINALURL|urlencode)", action.PostParams);
        EXPECT_EQ("http://8b.kz/", action.Referer);
        EXPECT_EQ(1, action.Regexes.size());
        {
            const ActionRegExp& regexp = action.Regexes[0];
            EXPECT_EQ("\"key\":\"((\\\\\"|[^\"])*)\"", regexp.Pattern);
            EXPECT_EQ(true, regexp.Required);
            EXPECT_EQ("surl:0", regexp.AssignVars);
        }
       
    }
}

