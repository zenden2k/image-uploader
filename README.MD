#Image Uploader
Free application for Windows that allows you to upload files to image/file hosting servers without necessity to visit their websites.

![Image Uploader main windows](https://lh5.googleusercontent.com/-GvexbrV8HUE/VQHXrxhom5I/AAAAAAAAD0c/JiH5OQ0XrfY/s0/26719_screenshot%2525202015-03-09%252520003_en.png)

[![Image Uploader main windows](https://lh4.googleusercontent.com/-D6wvBfMHonw/VQHgBb9ZFzI/AAAAAAAAD1s/Hq1uUei_C-s/s0/download-button-en.png)](http://zenden.ws/downloads/image-uploader-latest)

Supported operating systems: Windows 2000/XP/Vista/7/8/10 (also server versions)

##Main Features

###Uploading files to image hostings and file hostings
It can upload to more than [20 image hostings sites](http://zenden.ws/imageuploader_servers). After uploading, Image Uploader generates HTML code or BBCode for forums.

It is also possible to specify your own output format template.

###Grabbing Frames from a Video Clip
Image Uploader can grab frames from various types of video formats, e.g. AVI, MPG, VOB, WMV, MKV (it depends on DirectShow codecs you have installed on your system). It is recommended to install K-lite Codec Pack.

###Screen capturing
Image Uploader can do screenshots of entire screen or selected screen region.

Image Uploader is available in the following languages: English, Russian, Swedish, Romanian, Turkish, Serbian, Hrvatski, Farsi

##Compiling

You need Microsoft Visual C++ at least 9.0 version (Microsoft Visual Studio 2008 SP1). 
If you are using a newer MSVC version, you may experience some compilation errors, which can be solved easily (if you fix these errors, please provide a patch or pull request). But also you need to recompile ALL 3rd-party 
libraries against your compiler version (so I recommend to use Microsoft Visual Studio 2008 SP1 to avoid all these issues).


If you are using Visual Studio prior to 2010 version, you have to install Microsoft Windows 7 Platform SDK (or later) for enabling Windows 7 related features.
Also yoo need to install "msinttypes" headers. 

In order to compile Image Uploader you will need the following libraries:

WTL		    http://sourceforge.net/projects/wtl/

libcurl		http://curl.haxx.se/libcurl/

openssl		https://www.openssl.org

zlib		  http://www.zlib.net

pcre		  http://www.pcre.org

pcre++		http://www.daemon.de/PCRE

squirrel	http://squirrel-lang.org

sqplus		http://sourceforge.net/projects/sqplus/

ffmpeg		https://www.ffmpeg.org

tinyxml		http://sourceforge.net/projects/tinyxml/

zthreads	http://zthread.sourceforge.net

minizip		http://www.winimage.com/zLibDll/minizip.html

jsoncpp		https://github.com/open-source-parsers/jsoncpp

glog      https://github.com/google/glog

You can download ALL necessary libraries and headers (precompiled for MSVC 2008 SP1) by following link:

https://bintray.com/artifact/download/zenden/zenden-image-uploader/Contrib_MSVC_2008_SP1_and_sources_v1.3.0.7z


If you want to compile strmbase.lib by yourself you will need:

DirectX 8 SDK

DirectShow BaseClasses

If you receive errors related to "TOKEN_ELEVATION" or "ITaskBar3", it means you haven't installed latest Platform SDK. 

Also you can change WINVER value into "stdafx.h" file to 0x0600 or 0x0501 and compile Image Uploader 
without Windows 7 TaskBar features support.
