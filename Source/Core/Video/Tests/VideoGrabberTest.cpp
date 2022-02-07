#include <gtest/gtest.h>

#include "Core/UploadEngineList.h"
#include "Tests/TestHelpers.h"
#ifdef IU_ENABLE_FFMPEG
    #include "Core/Video/AvcodecFrameGrabber.h"
#endif
#include "Core/Video/DirectshowFrameGrabber.h"

class VideoGrabberTest : public ::testing::Test {
public:
    VideoGrabberTest() : fileName(TestHelpers::resolvePath("videofiles.txt")) {
        if (IuCoreUtils::FileExists(fileName)) {
            TestHelpers::getFileContents(fileName, videoFiles_);
        }
    }
protected:
    const std::string fileName;
    std::vector<std::string> videoFiles_;
};

#ifdef IU_ENABLE_FFMPEG
TEST_F(VideoGrabberTest, avcodecVideoGrabber)
{
    for (const auto& videoFile : videoFiles_) {
        AvcodecFrameGrabber grabber;
        ASSERT_TRUE(IuCoreUtils::FileExists(videoFile));
        bool res = grabber.open(videoFile);
        EXPECT_TRUE(res);
        if (res) {
            EXPECT_GT(grabber.duration(), 0);
            int64_t position = grabber.duration() / 2;
            ASSERT_TRUE(grabber.seek(position));
            AbstractVideoFrame* frame = grabber.grabCurrentFrame();
            EXPECT_TRUE(frame != nullptr);
            EXPECT_GT(frame->getWidth(), 0);
            EXPECT_GT(frame->getHeight(), 0);
        }
    }

}
#endif

TEST_F(VideoGrabberTest, directShowFrameGrabber)
{
    for (const auto& videoFile : videoFiles_) {
        DirectshowFrameGrabber grabber;
        ASSERT_TRUE(IuCoreUtils::FileExists(videoFile));
        bool res = grabber.open(videoFile);
        EXPECT_TRUE(res);
        if (res) {
            EXPECT_GT(grabber.duration(), 0);
            int64_t position = grabber.duration() / 2;
            ASSERT_TRUE(grabber.seek(position));
            AbstractVideoFrame* frame = grabber.grabCurrentFrame();
            EXPECT_TRUE(frame != nullptr);
            EXPECT_GT(frame->getWidth(), 0);
            EXPECT_GT(frame->getHeight(), 0);
        }
    }

}