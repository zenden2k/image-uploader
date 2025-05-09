#ifndef CORE_VIDEO_VIDEOUTILS_H
#define CORE_VIDEO_VIDEOUTILS_H

#include <vector>
#include <string>
#include <set>

class VideoUtils {
    public:
        static VideoUtils& instance();
        std::set<std::string> videoFilesExtensionsSet;
        std::set<std::string> audioFilesExtensionsSet;
        VideoUtils(const VideoUtils&) = delete;
        VideoUtils& operator=(const VideoUtils&) = delete;
    private:  
        VideoUtils();
};
#endif
