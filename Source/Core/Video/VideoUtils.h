#ifndef CORE_VIDEO_VIDEOUTILS_H
#define CORE_VIDEO_VIDEOUTILS_H

#include <vector>
#include <string>

class VideoUtils {
    public:
        static VideoUtils& instance();
        std::vector<std::string> videoFilesExtensions;
        VideoUtils(const VideoUtils&) = delete;
        VideoUtils& operator=(const VideoUtils&) = delete;
    private:  
        VideoUtils();
};
#endif