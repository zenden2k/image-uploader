/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "AvcodecFrameGrabber.h"
#include "AbstractImage.h"
#include "Core/Logging.h"
#include "Core/Utils/CoreUtils.h"

/*namespace AvCodec
{*/
    extern "C"
    {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    }
/*}
 
using namespace AvCodec;*/

class AvcodecVideoFrame: public AbstractVideoFrame {

public :
    AvcodecVideoFrame(AVFrame *frame, unsigned char *data, unsigned int dataSize, int64_t time, int width, int height) {
        time_ = time;
        frame_ = frame;
        data_ = data;
        dataSize_ = dataSize;
        width_ = width;
        height_ = height;
    }

    ~AvcodecVideoFrame() {
        frame_ = 0;
        //free(data_);
        //delete[] data_;
    }

    bool saveToFile(const std::string& fileName) const override {
        std::unique_ptr<AbstractImage> img(AbstractImage::createImage());
        if ( !img ) {
            return false;
        }
        if ( !img->loadFromRawData(AbstractImage::dfRGB888, width_, height_, data_, dataSize_, reinterpret_cast<void*>(frame_->linesize[0]))) {
            return false;
        }
        return img->saveToFile(fileName);
    }

    std::unique_ptr<AbstractImage> toImage() const override  {
        std::unique_ptr<AbstractImage> img(AbstractImage::createImage());
        if ( !img ) {
            return nullptr;
        }
        if ( !img->loadFromRawData(AbstractImage::dfRGB888, width_, height_, data_, dataSize_, reinterpret_cast<void*>(frame_->linesize[0]))) {
            return nullptr;
        }
        return img;
    }

 protected:
    unsigned char *data_;
    unsigned int dataSize_;
    AVFrame* frame_;
    DISALLOW_COPY_AND_ASSIGN(AvcodecVideoFrame);
};


class AvcodecFrameGrabberPrivate {
protected:
    int             videoStream;
        AVFormatContext *pFormatCtx;

        AVCodecContext  *pCodecCtx;
        AVCodec         *pCodec;
        AVFrame         *pFrame;
        AVFrame         *pFrameRGB;
        AVPacket        packet;
        AVFormatContext *ic;
        uint8_t         *buffer;
        int             frameFinished;
        int headerlen;
        struct SwsContext *img_convert_ctx;
        int             numBytes;
        int allocatedFrameWidth_;
        int allocatedFrameHeight_;
        size_t allocatedBufferSize_;
        uint64_t fileSize_;
        bool NeedStop;
        bool SeekToKeyFrame;
        bool seekByBytes;
        AvcodecVideoFrame *currentFrame_;
        int64_t duration_;
        static bool initialized_;
public:
        friend class AvcodecFrameGrabber;

        AvcodecFrameGrabberPrivate() {
            NeedStop = false;
            SeekToKeyFrame = true;
            seekByBytes = false;
            img_convert_ctx = NULL;
            currentFrame_ = NULL;
            fileSize_ = 0;
            buffer = 0;
            pFrameRGB = 0;
            pFrame = 0;
            pCodecCtx = 0;
            videoStream = 0;
            pFormatCtx = nullptr;
            pCodec = nullptr;
            frameFinished = 0;
            ic = nullptr;
            headerlen = 0;
            duration_ = 0;
            allocatedFrameWidth_ = 0;
            allocatedFrameHeight_ = 0;
            numBytes = 0;
            allocatedBufferSize_ = 0;
        }

          ~AvcodecFrameGrabberPrivate() {
              close();
          }


    bool open(const std::string& fileName) {
        int numOfFrames = 5;

        // Register all formats and codecs
        if (! initialized_) {
            av_register_all();

            initialized_ = true;
        }
        pFormatCtx = 0;
            //pFormatCtx = avformat_alloc_context();
        // Open video file
        int result = avformat_open_input(&pFormatCtx, fileName.c_str(), NULL, 0);
        if(result != 0) {
            LOG(ERROR) << "Cannot open input file (avformat_open_input failed), error code=" << result;
            return false; // Couldn't open file
        }

        /*AVInputFormat *inputFormat = */av_find_input_format( fileName.c_str() );

        // Retrieve stream information
        if(avformat_find_stream_info(pFormatCtx,0)<0) {
            LOG(ERROR) << "Couldn't find stream information.";
            return false; // Couldn't find stream information
        }
        fileSize_ = IuCoreUtils::getFileSize(fileName);
        // Dump information about file onto standard error
        av_dump_format(pFormatCtx, 0, fileName.c_str(), false);
        if ( pFormatCtx->iformat ) {
            seekByBytes= !!(pFormatCtx->iformat->flags & AVFMT_TS_DISCONT);
        }

        // Find the first video stream
        videoStream = -1;
        for (size_t i = 0; i<pFormatCtx->nb_streams; i++) {
            if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                 videoStream=i;
                 break;
            }
        }

        if(videoStream==-1) {
            LOG(ERROR) << "Could not find a video stream.";
            return false; 
        }
        // Get a pointer to the codec context for the video stream
        pCodecCtx = pFormatCtx->streams[videoStream]->codec;

        // Find the decoder for the video stream
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if( pCodec == NULL ){
            LOG(ERROR) << "Couldn't find codec.";
            return false; // Codec not found
        }

        pCodecCtx->refcounted_frames = 1;
       // Open codec
       if ( avcodec_open2(pCodecCtx, pCodec,0) < 0 ) {
            return false; // Could not open codec
        }

        // Hack to correct wrong frame rates that seem to be generated by some codecs
        if(pCodecCtx->time_base.num>1000 && pCodecCtx->time_base.den==1)
            pCodecCtx->time_base.den=1000;

       // Allocate video frame
        pFrame = av_frame_alloc();
        //avpicture_fill((AVPicture *)pFrame, decoded_yuv_frame, PIX_FMT_YUV420P, srcX , srcY);

       // Allocate an AVFrame structure
        pFrameRGB = av_frame_alloc();
        
        if ( pFrameRGB == NULL ){
            return false;
        }
        memset( pFrameRGB->linesize, 0,sizeof(pFrameRGB->linesize));

       // Determine required buffer size and allocate buffer
        allocatedFrameWidth_ = pCodecCtx->width;
        allocatedFrameHeight_ = pCodecCtx->height;
        numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height) + 64;
        buffer=/*(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));**/new uint8_t[numBytes];
        allocatedBufferSize_ = numBytes;
        memset(buffer, 0, numBytes);
        headerlen = sprintf((char *) buffer, "P6\n%d %d\n255\n", pCodecCtx->width,pCodecCtx->height);


       // Assign appropriate parts of buffer to image planes in pFrameRGB
       avpicture_fill((AVPicture *)pFrameRGB, buffer+headerlen, AV_PIX_FMT_RGB24,
        pCodecCtx->width, pCodecCtx->height);

       // Read frames and save first five frames to disk

       int64_t dur = pFormatCtx->streams[videoStream]->duration;
       int64_t dur2 = pFormatCtx->duration;
       if(dur < 0) dur = dur2;

        //cb->Width = pCodecCtx->width;
        //cb->Height =  pCodecCtx->height;

        AVRational ss =pFormatCtx->streams[videoStream]->time_base;
        //int64_t step = dur/numOfFrames;
         //CString result;//AVSEEK_FLAG_FRAME
         int64_t sec=dur/ss.den;
         int64_t min = sec/60;
         sec = sec%60;
         //int64_t hr = min/60;
         min=min%60;
         //std::string s;
         ic = pFormatCtx;
         int64_t full_sec =0; //длина видео, в секундах

         if (ic->duration != AV_NOPTS_VALUE) {
             int hours, mins, secs, us;
             secs = static_cast<int>(ic->duration / AV_TIME_BASE);
             full_sec = secs;
             us = ic->duration % AV_TIME_BASE;
             mins = secs / 60;
             secs %= 60;
             hours = mins / 60;
             mins %= 60;
             /*CString s;
             s.Format(_T("%02d:%02d:%02d.%02d"), hours, mins, secs,
                 (100 * us) / AV_TIME_BASE);*/
         }


         //step = full_sec / numOfFrames;

         AVDictionaryEntry  *tag = NULL;
         while ((tag=av_dict_get (pFormatCtx->streams[videoStream]->metadata,"",tag,AV_DICT_IGNORE_SUFFIX )) !=0 ) {
         }

        int start_time = 3000;
        int64_t timestamp;
        timestamp = start_time;

        int64_t my_start_time;
        int64_t seek_target;

        duration_ = ic->duration;
        my_start_time = static_cast<uint64_t>((double)ic->duration / numOfFrames*1.6);

         AVRational rat = {1, AV_TIME_BASE};
         //my_start_time = av_rescale_q(my_start_time, rat, pFormatCtx->streams[videoStream]->time_base);
        seek_target = av_rescale_q(my_start_time, rat, pFormatCtx->streams[videoStream]->time_base);
         if ( seekByBytes ){
             uint64_t size=  avio_size(pFormatCtx->pb);
             seek_target = static_cast<uint64_t>((double)my_start_time / ic->duration * size);
         }
         if ( avformat_seek_file(pFormatCtx, videoStream, 0, seek_target, seek_target, seekByBytes?AVSEEK_FLAG_BYTE:0)  < 0 ) {
             LOG(ERROR) << "avformat_seek_file failed to seek to position "<<seek_target<< " seekByBytes="<<seekByBytes;;
         }
         avcodec_flush_buffers(pCodecCtx) ;

         avcodec_flush_buffers(pCodecCtx) ;

         while(av_read_frame(pFormatCtx, &packet)>=0) {
             if(packet.stream_index==videoStream) {

                 frameFinished = false;
                int res =0;
                av_frame_unref(pFrame);
                 if ( (res = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet)) < 0 ) {
                    LOG(ERROR) << "avcodec_decode_video2() result="<<res;
                 }

                 if(frameFinished) {
                     av_free_packet(&packet);
                     break;
                 }
             }

             // Free the packet that was allocated by av_read_frame
             av_free_packet(&packet);
             // memset( &packet, 0, sizeof( packet ) );
         }

         return true;
    }

    void close() {
        // Free the RGB image
        delete[] buffer;

        if ( pFrameRGB ) {
             av_frame_free(&pFrameRGB);
        }
        if ( img_convert_ctx ) {
            sws_freeContext(img_convert_ctx);
        }

        if ( pFrame ) {
            // Free the YUV frame
                av_frame_free(&pFrame);
        }

        if ( pCodecCtx ) {
            // Close the codec
            avcodec_close(pCodecCtx);
        }

        if ( pFormatCtx ) {
            // Close the video file
            avformat_close_input (&pFormatCtx);    
        }
    }
    /*void write_frame_to_file(AVFrame* frame, int width, int height, int iframe)
    {
        //int i = AV_PIX_FMT_YUV420P;
        FILE* outfile;
        char filename[32];

        // Open file
        sprintf(filename, "d:\\frame%d.yuv", iframe);
        outfile = fopen(filename, "wb");
        if (outfile == NULL) {
            return;
        }

        // Write pixel data
        fwrite(frame->data[0], 1, frame->linesize[0]*height,outfile);
        fwrite(frame->data[1], 1, frame->linesize[1]*height/2,outfile);
        fwrite(frame->data[2],1, frame->linesize[2]*height/2,outfile);
        /*for (y = 0; y < height; ++y) {
            fwrite(frame->data[0]+y*frame->linesize[0], 1, frame->linesize[0],outfile);
            
        }
        for (y = 0; y < height; ++y) {
            fwrite(frame->data[1]+y*frame->linesize[1], 1, frame->linesize[1],outfile);

        }
        for (y = 0; y < height; ++y) {
            fwrite(frame->data[0]+y*frame->linesize[2], 1, frame->linesize[2],outfile);

        }*

        // Close file
        fclose(outfile);
    }

    void saveFramePPM(AVFrame *frame, int width, int height, int iFrame)
    {
        FILE *pFile;
        char szFilename[32];
        int  y;

        // Open file
        sprintf(szFilename, "d:\\aframe%d.ppm", iFrame);
        pFile=fopen(szFilename, "wb");
        if(pFile==NULL)
            return;

        // Write header
        fprintf(pFile, "P6\n%d %d\n255\n", width, height);

        // Write pixel data
        for(y=0; y<height; y++)
            fwrite(frame->data[0]+y*frame->linesize[0], 1, width*3, pFile);

        // Close file
        fclose(pFile);
    }*/



    bool seek(int64_t time) {  
        {
            if (allocatedFrameWidth_ != pCodecCtx->width || allocatedFrameHeight_ != pCodecCtx->height) {  
                numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height) + 64;
                if (allocatedBufferSize_ < numBytes) {
                    delete[] buffer;
                    buffer = nullptr;
                    buffer = new uint8_t[numBytes];
                    allocatedBufferSize_ = numBytes;
                    memset(buffer, 0, numBytes);
                }
                allocatedFrameWidth_ = pCodecCtx->width;
                allocatedFrameHeight_ = pCodecCtx->height;
            }
            avpicture_fill((AVPicture *)pFrameRGB, buffer+headerlen, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
            if(NeedStop) {
                //break;
            }
          
            

            SeekToKeyFrame = true;

            int64_t seek_target = time;
            if(seek_target==0) seek_target = 1;

            AVRational rat = {1, AV_TIME_BASE};
            //seekByBytes = true;
            if ( seekByBytes ) {
                uint64_t size=  avio_size(pFormatCtx->pb);
                seek_target = static_cast<int64_t>((double)seek_target / ic->duration * size);
            } else {
                seek_target = av_rescale_q(seek_target, rat, pFormatCtx->streams[videoStream]->time_base);
            }

            /*int64_t seek_min= static_cast<int64_t>(seek_target*0.9);
            int64_t seek_max = static_cast<int64_t>(/INT64_MAX*seek_target*1.1);*/

            if ( avformat_seek_file(pFormatCtx, videoStream,0, seek_target, seek_target, seekByBytes?AVSEEK_FLAG_BYTE:0) < 0  )  {
                LOG(WARNING) << "avformat_seek_file failed to seek to position "<<seek_target << " seekByBytes="<<seekByBytes;
            }

            avcodec_flush_buffers(pCodecCtx) ;
            AVPixelFormat pixelFormat = 
#ifdef IU_QT
                AV_PIX_FMT_RGB24;
#else
                
                AV_PIX_FMT_BGR24;
                //PIX_FMT_RGB24 ;
#endif
            while (av_read_frame(pFormatCtx, &packet)>=0) {
                // Is this a packet from the video stream?
                if(packet.stream_index==videoStream) {
                    frameFinished = false;
                    av_frame_unref(pFrame);
                    if ( avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet) < 0 ) {
                        LOG(ERROR) << "avcodec_decode_video2() failed";
                    }

                    // Did we get a video frame?
                    if(frameFinished) {

                        //write_frame_to_file(pFrame, pCodecCtx->width,pCodecCtx->height,rand()%100);
                        // Convert the image into YUV format that SDL uses
                        if(img_convert_ctx == NULL)
                        {
                            int w = pCodecCtx->width;
                            int h = pCodecCtx->height;
                            //pCodecCtx->
                            int dstWidth = w;
                            int dstHeight = h;
                            
                            img_convert_ctx = sws_getCachedContext(img_convert_ctx, w, h, pCodecCtx->pix_fmt, dstWidth, dstHeight,pixelFormat , /*SWS_BICUBIC*/SWS_LANCZOS,
                                NULL, NULL, NULL);
                            if(img_convert_ctx == NULL) {
                                LOG(ERROR) << "Cannot initialize the conversion context!";
                                av_free_packet(&packet);
                                return false;
                            }
                        }

                        sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                        int64_t display_time = static_cast<int64_t>((double)av_rescale_q(pFrame->pkt_pts, pFormatCtx->streams[videoStream]->time_base, rat) / (double)AV_TIME_BASE);
                         //pFrame->pkt_pts/ (double)AV_TIME_BASE;
                         if ( display_time <= 0 ) {
                             display_time = static_cast<int64_t>((double)time/ (double)AV_TIME_BASE);
                         }
                             
                         if ( display_time < 0 ) {
                             display_time = 0;
                         }

                        
                        currentFrame_ = new AvcodecVideoFrame(pFrameRGB, pFrameRGB->data[0],numBytes, display_time,pCodecCtx->width, pCodecCtx->height);
                        av_free_packet(&packet);
                        break;
                    }
                }

                // Free the packet that was allocated by av_read_frame
                av_free_packet(&packet);
                memset( &packet, 0, sizeof( packet ) );
            }
            
        }
        return true;
    }

    AbstractVideoFrame* grabCurrentFrame() {
        return currentFrame_;
    }

};

 bool AvcodecFrameGrabberPrivate::initialized_ = false;

AvcodecFrameGrabber::AvcodecFrameGrabber():  d_ptr(new AvcodecFrameGrabberPrivate){

}

AvcodecFrameGrabber::~AvcodecFrameGrabber()
{
    delete d_ptr;
}

bool AvcodecFrameGrabber::open(const std::string& fileName) {
    return d_ptr->open(fileName);
}

bool AvcodecFrameGrabber::seek(int64_t time) {
    return d_ptr->seek(time);
}

AbstractVideoFrame* AvcodecFrameGrabber::grabCurrentFrame() {
    return d_ptr->grabCurrentFrame();
}

int64_t AvcodecFrameGrabber::duration() {
    return d_ptr->duration_;
}

//bool NeedStop;

std::string timestamp_to_str(int64_t duration,int64_t units) {
    int hours, mins, secs, us;
    secs = static_cast<int>(duration / units);
    us = static_cast<int>(duration % units);
    mins = secs / 60;
    secs %= 60;
    hours = mins / 60;
    mins %= 60;
    char buffer[100];
    sprintf(buffer, "%02d:%02d:%02d.%02d", hours, mins, secs, (int)((100 * us) / units));
    return buffer;
}

void memswap(void *p1, void *p2, size_t count) {
    char tmp, *c1, *c2;

    c1 = (char *)p1;
    c2 = (char *)p2;

    while (count--) 
    {
        tmp = *c2;
        *c2++ = *c1;
        *c1++ = tmp;
    }
};

/*void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame,int headerlen,const std::string& Title, CSampleGrabberCB * cb, int64_t target_frame) {
    cb->Grab = true;

    int dataSize = width*height * 3;
    uint8_t * data = new uint8_t[dataSize+100000];
    int lineSizeInBytes = pFrame->linesize[0];
    for ( int y=height-1; y>=0; y--) {
        memcpy(data+(height-y-1)*lineSizeInBytes, pFrame->data[0]+y*lineSizeInBytes,lineSizeInBytes );
    }

    cb->BufferCB((double)target_frame, data, dataSize);
    delete[] data;
}*/





