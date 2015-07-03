#include "VideoUtils.h"
#include "Core/Utils/StringUtils.h"

VideoUtils& VideoUtils::Instance()
{
    static VideoUtils theSingleInstance;
    return theSingleInstance;
} 

VideoUtils::VideoUtils(){
    std::string formats = "asf;avi;mpeg;mpg;mp2;divx;vob;flv;wmv;mkv;mp4;ts;mov;mpeg2ts;3gp;mpeg1;mpeg2;mpeg4;mv4;rmvb;qt;hdmov;divx;m4v;ogv;m2v;webm";

    IuStringUtils::Split(formats, ";", videoFilesExtensions, -1);
}