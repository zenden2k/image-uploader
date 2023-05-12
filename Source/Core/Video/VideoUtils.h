#ifndef CORE_VIDEO_VIDEOUTILS_H
#define CORE_VIDEO_VIDEOUTILS_H

#include <vector>
#include <string>

#include "atlheaders.h"

class VideoUtils {
    public:
        static VideoUtils& instance();
        std::vector<std::string> videoFilesExtensions;
        VideoUtils(const VideoUtils&) = delete;
        VideoUtils& operator=(const VideoUtils&) = delete;
        CString prepareVideoDialogFilters() const;
    private:  
        VideoUtils();
};
#endif