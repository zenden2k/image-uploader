#include "ScreenRecorder.h"

#include <boost/format.hpp>


#include "Core/Logging.h"

//using namespace std;

/* initialize the resources*/
ScreenRecorder::ScreenRecorder() {

    /*av_register_all();
    avcodec_register_all();*/
    avdevice_register_all();
    LOG(ERROR) << "\nall required functions are registered successfully";
}

/* uninitialize the resources */
ScreenRecorder::~ScreenRecorder() {
    avformat_close_input(&pAVFormatContext);
    if (!pAVFormatContext) {
        LOG(ERROR) << "\nfile closed sucessfully";
    } else {
        LOG(ERROR) << "\nunable to close the file";
    }

    avformat_free_context(pAVFormatContext);
    if (!pAVFormatContext) {
        LOG(ERROR) << "\navformat free successfully";
    } else {
        LOG(ERROR) << "\nunable to free avformat context";
    }
    avcodec_free_context(&pAVCodecInputContext);
    avcodec_free_context(&outAVCodecContext);
}

/* function to capture and store data in frames by allocating required memory and auto deallocating the memory.   */
int ScreenRecorder::CaptureVideoFrames() {
    int flag;
    int frameFinished;
    //when you decode a single packet, you still don't have information enough to have a frame [depending on the type of codec, some of them //you do], when you decode a GROUP of packets that represents a frame, then you have a picture! that's why frameFinished will let //you know you decoded enough to have a frame.

    int frame_index = 0;
    int enc_packet_counter = 0;
    value = 0;

    AVPacket pAVPacket; // = (AVPacket*)av_malloc(sizeof(AVPacket));
    av_init_packet(&pAVPacket);

    pAVFrame = av_frame_alloc();
    if (!pAVFrame) {
        LOG(ERROR) << "\nunable to release the avframe resources";
        return 1;
    }

    outFrame = av_frame_alloc(); //Allocate an AVFrame and set its fields to default values.
    if (!outFrame) {
        LOG(ERROR) << "\nunable to release the avframe resources for outframe";
        return 1;
    }

    int video_outbuf_size;
    int nbytes = av_image_get_buffer_size(outAVCodecContext->pix_fmt, outAVCodecContext->width,
                                          outAVCodecContext->height, 32);
    uint8_t* video_outbuf = (uint8_t*)av_malloc(nbytes);
    if (video_outbuf == NULL) {
        LOG(ERROR) << "\nunable to allocate memory";
        return 1;
    }

    // Setup the data pointers and linesizes based on the specified image parameters and the provided array.
    value = av_image_fill_arrays(outFrame->data, outFrame->linesize, video_outbuf, AV_PIX_FMT_YUV420P,
                                 outAVCodecContext->width, outAVCodecContext->height, 1);
    // returns : the size in bytes required for src
    if (value < 0) {
        LOG(ERROR) << "\nerror in filling image array";
    }

    SwsContext* swsCtx_;

    // Allocate and return swsContext.
    // a pointer to an allocated context, or NULL in case of error
    // Deprecated : Use sws_getCachedContext() instead.

    swsCtx_ = sws_getContext(pAVCodecInputContext->width,
                             pAVCodecInputContext->height,
                             pAVCodecInputContext->pix_fmt,
                             outAVCodecContext->width,
                             outAVCodecContext->height,
                             outAVCodecContext->pix_fmt,
                             SWS_BICUBIC, NULL, NULL, NULL);


    int ii = 0;
    int no_frames = 200;
    LOG(ERROR) << "\nenter No. of frames to capture : ";
    //cin>>no_frames;
    int64_t startTime = av_gettime();
    AVPacket outPacket;
    int j = 0;
    char strBuf[256] = "\0";

    int got_picture;
    int ret;

    while (av_read_frame(pAVFormatContext, &pAVPacket) >= 0) {
        //LOG(ERROR) << "av_read_frame success";
        if (pAVPacket.stream_index == VideoStreamIndx) {
            frameFinished = false;
            ret = avcodec_send_packet(pAVCodecInputContext, &pAVPacket);
            if (ret < 0) {
                LOG(WARNING) << "Error sending a packet for decoding";
            } else {

                ret = avcodec_receive_frame(pAVCodecInputContext, pAVFrame);
                av_packet_unref(&pAVPacket);

                if (ret == AVERROR(EAGAIN)) {
                    //LOG(ERROR) << "EAGAIN";
                    continue;
                    //return false;
                } else if (ret == AVERROR_EOF) {
                    //LOG(ERROR) << "AVERROR_EOF";
                    break;
                } else if (ret < 0) {
                    break;
                    //throw FrameGrabberException("Error during decoding");
                }
                frameFinished = true;
                if (frameFinished) // Frame successfully decoded :)
                {
                    if (ii++ == no_frames) {
                        break;
                    }
                    //LOG(ERROR) << "frameFinished";
                    outFrame->format = /*AV_PIX_FMT_YUV420P*/ outAVCodecContext->pix_fmt;
                    outFrame->width = pAVFrame->width;
                    outFrame->height = pAVFrame->height;

                    int64_t now = av_gettime() - startTime;
                    AVRational time_base = av_make_q(1, 90000);
                    outFrame->pts = av_rescale_q(now, AVRational({1, 1000000}), time_base);
                    outFrame->pkt_dts = outFrame->pts;

                    //outFrame->pts = av_frame_get_best_effort_timestamp(outFrame);
                    //outFrame->pkt_pts = pAVFrame->pkt_pts;
                    //outFrame->dts = pAVFrame->dts;
                    int res2 = sws_scale(swsCtx_, pAVFrame->data, pAVFrame->linesize, 0, pAVCodecInputContext->height,
                                         outFrame->data, outFrame->linesize);
                    /*if (res2 < 0) {
                        av_strerror(res2, strBuf, sizeof(strBuf));
                        LOG(ERROR) << "\navcodec_encode_video2 failed " << res2 << std::endl << strBuf;
                    }*/
                    av_init_packet(&outPacket);
                    outPacket.data = NULL; // packet data will be allocated by the encoder
                    outPacket.size = 0;

                    outPacket.pts = outFrame->pts;
                    outPacket.dts = outFrame->pts;
                    //outPacket.duration = av_rescale_q(1, outAVCodecContext->time_base, outVideoStream->time_base);
                    /*outPacket.pts = av_rescale_q(enc_packet_counter, outAVCodecContext->time_base, outVideoStream->time_base);
                    if (outPacket.dts != AV_NOPTS_VALUE)
                        outPacket.dts = av_rescale_q(enc_packet_counter, outAVCodecContext->time_base, outVideoStream->time_base);
                    outPacket.dts = av_rescale_q(enc_packet_counter, outAVCodecContext->time_base, outVideoStream->time_base);
                    outPacket.duration = av_rescale_q(1, outAVCodecContext->time_base, outVideoStream->time_base);


                    outFrame->pts = av_rescale_q(enc_packet_counter, outAVCodecContext->time_base, outVideoStream->time_base);
                    outFrame->pkt_duration = av_rescale_q(enc_packet_counter, outAVCodecContext->time_base, outVideoStream->time_base);*/
                    enc_packet_counter++;

                    //outPacket.pts = outFrame->pts;
                    ret = avcodec_send_frame(outAVCodecContext, outFrame);
                    if (ret < 0) {
                        LOG(ERROR) << "avcodec_send_frame error";
                    }

                    while (ret >= 0) {
                        ret = avcodec_receive_packet(outAVCodecContext, &outPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        else if (ret < 0) {
                            LOG(ERROR) << "Error during encoding";
                        }

                        //if (got_picture)
                        {
                            //LOG(ERROR) << "got_picture";

                            av_packet_rescale_ts(&outPacket, /*pAVCodecInputContext->time_base*/time_base,
                                                 outVideoStream->time_base);
                            /*if (outPacket.pts != AV_NOPTS_VALUE)
                                outPacket.pts = av_rescale_q(outPacket.pts, video_st->codec->time_base, video_st->time_base);
                            if (outPacket.dts != AV_NOPTS_VALUE)
                                outPacket.dts = av_rescale_q(outPacket.dts, video_st->codec->time_base, video_st->time_base);*/

                            LOG(WARNING) << boost::format("Write frame %3d (size= %2d)\n") % (j++) % (outPacket.size / 1000);
                            if (av_write_frame(outAVFormatContext, &outPacket) != 0) {
                                LOG(ERROR) << "\nerror in writing video frame";
                            }

                            av_packet_unref(&outPacket);
                        } // got_picture


                    }


                    //
                    //printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
                    //fwrite(pkt->data, 1, pkt->size, outfile);
                    av_packet_unref(&outPacket);
                    /*int res = avcodec_encode_video2(outAVCodecContext, &outPacket, outFrame, &got_picture);
                    if (res < 0) {
                        av_strerror(res, strBuf, sizeof(strBuf));
                        LOG(ERROR) << "\navcodec_encode_video2 failed " << res << std::endl << strBuf;
                    }*/


                    //av_packet_unref(&outPacket);
                }
                /*else {
                    LOG(ERROR) << "frame NOT finished";
                    // frameFinished
                }*/
                //break;


                /*value = avcodec_decode_video2(pAVCodecContext, pAVFrame, &frameFinished, &pAVPacket);
                if( value < 0)
                {
                    LOG(ERROR) <<"unable to decode video";
                }*/


            }
        } // End of while-loop


    }
    value = av_write_trailer(outAVFormatContext);
    if (value < 0) {
        LOG(ERROR) << "\nerror in writing av trailer";
        return 1;
    }
    //THIS WAS ADDED LATER
    av_free(video_outbuf);

}

/* establishing the connection between camera or screen through its respective folder */
int ScreenRecorder::openCamera() {
    /*offsetX = 300;
    offsetY = 200;
    width = 640;
    height = 480;*/
    width = 2560;
    height = 1440;
    std::string video_size = std::to_string(width) + "x" + std::to_string(height);
    value = 0;
    options = NULL;
    pAVFormatContext = NULL;

    pAVFormatContext = avformat_alloc_context(); //Allocate an AVFormatContext.
    /*
    
    X11 video input device.
    To enable this input device during configuration you need libxcb installed on your system. It will be automatically detected during configuration.
    This device allows one to capture a region of an X11 display. 
    refer : https://www.ffmpeg.org/ffmpeg-devices.html#x11grab
    */
    /* current below is for screen recording. to connect with camera use v4l2 as a input parameter for av_find_input_format */
    pAVInputFormat = av_find_input_format("gdigrab");
    // ddagrab available since ffmpeg 6.0
    //pAVInputFormat = av_find_input_format("ddagrab");
    //pAVInputFormat = av_find_input_format("ddagrab=0,hwdownload,format=bgra");

    //AVDictionary* options = nullptr;
    //av_dict_set(&options, "framerate", "30", 0);
    //av_dict_set(&options, "video_size", "300x200", 0);

    //value = avformat_open_input(&pAVFormatContext, ":0.0+300,200", pAVInputFormat, NULL);

    value = av_dict_set(&options, "offset_x", std::to_string(offsetX).c_str(), 0);
    value = av_dict_set(&options, "offset_y", std::to_string(offsetY).c_str(), 0);
    value = av_dict_set(&options, "video_size", video_size.c_str(), 0);
    value = av_dict_set(&options, "rtbufsize", "100M", 0);


    /*value = av_dict_set(&options, "width", "600", 0);
    value = av_dict_set(&options, "height", "500", 0);*/
    /* set frame per second */
    value = av_dict_set(&options, "framerate", "30", 0);
    if (value < 0) {
        LOG(ERROR) << "\nerror in setting dictionary value";
        return 1;
    }

    /*value = av_dict_set(&options, "preset", "ultrafast", 0);
    if (value < 0) {
        LOG(ERROR) << "\nerror in setting preset values";
        return 1;
    }*/

    value = avformat_open_input(&pAVFormatContext, "desktop", pAVInputFormat, &options);
    if (value != 0) {
        LOG(ERROR) << "\nerror in opening input device";
        return 1;
    }

    value = avformat_find_stream_info(pAVFormatContext,NULL);
    if (value < 0) {
        LOG(ERROR) << "\nunable to find the stream information";
        return 1;
    }

    VideoStreamIndx = -1;

    /* find the first video stream index . Also there is an API available to do the below operations */
    for (int i = 0; i < pAVFormatContext->nb_streams; i++) {
        // find video stream posistion/index.
        if (pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            VideoStreamIndx = i;
            break;
        }

    }

    if (VideoStreamIndx == -1) {
        LOG(ERROR) << "\nunable to find the video stream index. (-1)";
        return 1;
    }


    // assign pAVFormatContext to VideoStreamIndx
    //pAVCodecContext = pAVFormatContext->streams[VideoStreamIndx]->codec;

    pAVCodec = avcodec_find_decoder(pAVFormatContext->streams[VideoStreamIndx]->codecpar->codec_id);
    if (pAVCodec == NULL) {
        LOG(ERROR) << "\nunable to find the decoder";
        return 1;
    }

    pAVCodecInputContext = avcodec_alloc_context3(pAVCodec);

    if (avcodec_parameters_to_context(pAVCodecInputContext, pAVFormatContext->streams[VideoStreamIndx]->codecpar) < 0) {
        LOG(ERROR) << "\navcodec_parameters_to_context error";
        return 1;
    }

    value = avcodec_open2(pAVCodecInputContext, pAVCodec, NULL);
    //Initialize the AVCodecContext to use the given AVCodec.
    if (value < 0) {
        LOG(ERROR) << "\nunable to open the av codec";
        return 1;
    }
    return 0;
}

/* initialize the video output file and its properties  */
int ScreenRecorder::initOutputFile() {
    outAVFormatContext = NULL;
    value = 0;
    output_file = "d:\\iu_output.mp4";

    avformat_alloc_output_context2(&outAVFormatContext, NULL, NULL, output_file);
    if (!outAVFormatContext) {
        LOG(ERROR) << "\nerror in allocating av format output context";
        return 1;
    }

    /* Returns the output format in the list of registered output formats which best matches the provided parameters, or returns NULL if there is no match. */
    output_format = av_guess_format(NULL, output_file, NULL);
    if (!output_format) {
        LOG(ERROR) << "\nerror in guessing the video format. try with correct format";
        return 1;
    }

    outAVCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!outAVCodec) {
        LOG(ERROR) << "\nerror in finding the av codecs. try again with correct codec";
        return 1;
    }

    outVideoStream = avformat_new_stream(outAVFormatContext, outAVCodec);
    if (!outVideoStream) {
        LOG(ERROR) << "\nerror in creating a av format new stream";
        return 1;
    }

    outAVCodecContext = avcodec_alloc_context3(outAVCodec);
    if (!outAVCodecContext) {
        LOG(ERROR) << "\nerror in allocating the codec contexts";
        return 1;
    }

    /* set property of the video file */
    //outAVCodecContext = video_st->codec;
    outAVCodecContext->codec_id = AV_CODEC_ID_H264; // AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    //outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    outAVCodecContext->framerate = AVRational{30, 1};

    outAVCodecContext->bit_rate = (width * height * 30 * 0.08);/*4000000*/ // 2500000
    outAVCodecContext->width = width;
    outAVCodecContext->height = height;
    /*outAVCodecContext->gop_size = 3;
    outAVCodecContext->max_b_frames = 2;*/
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = 30; // 15fps
    //outAVCodecContext->qua
    if (outAVCodecContext->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(outAVCodecContext->priv_data, "preset", "ultrafast", 0);
    }

    /* Some container formats (like MP4) require global headers to be present
       Mark the encoder so that it behaves accordingly. */

    if (outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        outAVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    value = avcodec_open2(outAVCodecContext, outAVCodec, NULL);
    if (value < 0) {
        char strBuf[256] = "\0";
        av_strerror(value, strBuf, sizeof(strBuf));
        LOG(ERROR) << "\nerror in opening the avcodec " << value << " " << std::endl << strBuf;
        return 1;
    }

    value = avcodec_parameters_from_context(outVideoStream->codecpar, outAVCodecContext);
    if (value < 0) {
        char strBuf[256] = "\0";
        av_strerror(value, strBuf, sizeof(strBuf));
        LOG(ERROR) << "\nerror in avcodec_parameters_from_context " << value << " " << std::endl << strBuf;
        return 1;
    }

    //outAVCodecContext->codec

    /* create empty video file */
    if (!(outAVFormatContext->flags & AVFMT_NOFILE)) {
        if (avio_open2(&outAVFormatContext->pb, output_file, AVIO_FLAG_WRITE,NULL, NULL) < 0) {
            LOG(ERROR) << "\nerror in creating the video file";
            return 1;
        }
    }

    if (!outAVFormatContext->nb_streams) {
        LOG(ERROR) << "\noutput file dose not contain any stream";
        return 1;
    }

    /* imp: mp4 container or some advanced container file required header information*/
    value = avformat_write_header(outAVFormatContext, &options);
    if (value < 0) {
        char strBuf[256] = "\0";
        av_strerror(value, strBuf, sizeof(strBuf));
        LOG(ERROR) << "\nerror in writing the header context " << value << std::endl << strBuf;
        return 1;
    }

    /*
    // uncomment here to view the complete video file informations
    LOG(ERROR) <<"\n\nOutput file information :\n\n";
    av_dump_format(outAVFormatContext , 0 ,output_file ,1);
    */
    return 0;
}


