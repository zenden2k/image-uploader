#ifndef CORE_VIDEO_VIDEOUTILS_H
#define CORE_VIDEO_VIDEOUTILS_H

#include <vector>
#include <string>

class VideoUtils {
	public:
		static VideoUtils& Instance();
		std::vector<std::string> videoFilesExtensions;
	private:  
		VideoUtils();
		VideoUtils(VideoUtils& root){}
		VideoUtils& operator=(VideoUtils&){}

};
#endif