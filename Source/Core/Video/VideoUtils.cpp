#include "VideoUtils.h"

#include "Core/Utils/StringUtils.h"
#include "Core/CommonDefs.h"

namespace {

constexpr auto IU_VIDEOFILES_EXTENSIONS = "asf;avi;mpeg;mpg;mp2;divx;vob;flv;wmv;mkv;mp4;ts;mov;mpeg2ts;3gp;mpeg1;mpeg2;mpeg4;mv4;rmvb;qt;hdmov;divx;m4v;ogv;m2v;webm";
constexpr auto IU_AUDIOFILES_EXTENSIONS = "aa;aac;ac3;adx;ahx;aiff;ape;asf;asx;au;snd;aud;dmf;dts;dxd;flac;la;m4a;mmf;mod;mp1;mp2;mp3;mp4;mpc;ofr;oga;ogg;opus;pac;pxd;ra;rka;sb0;shn;tta;voc;vqf;wav;wma;wv;xm;cd;mqa;mid";
constexpr auto IU_EXTENSIONS_LIST_SEPARATOR = ";";

}

VideoUtils& VideoUtils::instance()
{
    static VideoUtils theSingleInstance;
    return theSingleInstance;
} 

VideoUtils::VideoUtils(){
    IuStringUtils::Split(IU_VIDEOFILES_EXTENSIONS, IU_EXTENSIONS_LIST_SEPARATOR, videoFilesExtensions, -1);
    IuStringUtils::Split(IU_AUDIOFILES_EXTENSIONS, IU_EXTENSIONS_LIST_SEPARATOR, audioFilesExtensions, -1);
}
