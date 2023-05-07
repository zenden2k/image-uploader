#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>

#define __STDC_CONSTANT_MACROS

//FFMPEG LIBRARIES
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"

#include "libavdevice/avdevice.h"

#include "libavfilter/avfilter.h"
//#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

#include "libavformat/avformat.h"
#include "libavformat/avio.h"

// libav resample

#include "libavutil/opt.h"
#include "libavutil/common.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/file.h"

// lib swresample

#include "libswscale/swscale.h"

}



class ScreenRecorder
{
private:
	int offsetX=0, offsetY=0, width=0, height=0;
	 AVInputFormat *pAVInputFormat = nullptr;
	const AVOutputFormat *output_format = nullptr;

	AVCodecContext *pAVCodecInputContext = nullptr;

	AVFormatContext *pAVFormatContext = nullptr;

	AVFrame *pAVFrame = nullptr;
	AVFrame *outFrame = nullptr;

	const AVCodec *pAVCodec = nullptr;
	const AVCodec *outAVCodec = nullptr;

	AVPacket *pAVPacket = nullptr;

	AVDictionary *options = nullptr;

	AVOutputFormat *outAVOutputFormat = nullptr;
	AVFormatContext *outAVFormatContext = nullptr;
	AVCodecContext *outAVCodecContext = nullptr;

	AVStream *outVideoStream = nullptr;
	AVFrame *outAVFrame = nullptr;

	const char *dev_name;
	const char *output_file;

	double video_pts;

	int out_size;
	int codec_id;
	int value;
	int VideoStreamIndx;

public:

	ScreenRecorder();
	~ScreenRecorder();

	/* function to initiate communication with display library */
	int openCamera();
	int initOutputFile();
	int CaptureVideoFrames();

};

#endif
