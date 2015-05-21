#Image Uploader
Free program for Windows that allows you to upload images, screenshots, or any other files to over 30 different file hosting services. 

![Image Uploader main window](https://lh5.googleusercontent.com/-GvexbrV8HUE/VQHXrxhom5I/AAAAAAAAD0c/JiH5OQ0XrfY/s0/26719_screenshot%2525202015-03-09%252520003_en.png)

[![Image Uploader main window](https://lh4.googleusercontent.com/-D6wvBfMHonw/VQHgBb9ZFzI/AAAAAAAAD1s/Hq1uUei_C-s/s0/download-button-en.png)](http://zenden.ws/imageuploader_downloads)

Supported operating systems: Windows 8/7/Vista/XP SP2 (also server versions starting with Windows Server 2003 SP1)

##Main Features

###Uploading files to image hostings and file hostings
It can upload to more than [20 image hostings sites](http://zenden.ws/imageuploader_servers). After uploading, Image Uploader generates HTML code or BBCode for forums.

It is also possible to specify your own output format template.

###Grabbing Frames from a Video Clip
Image Uploader can grab frames from various types of video formats, e.g. AVI, MPG, VOB, WMV, MKV (it depends on DirectShow codecs you have installed on your system). It is recommended to install K-lite Codec Pack.

###Screen capturing
Image Uploader can do screenshots of entire screen or selected screen region.

Image Uploader is available in the following languages: English, Russian, Swedish, Romanian, Turkish, Serbian, Hrvatski, Farsi

##Console Utility

![Image Uploader Console Utility running on Ubuntu](https://lh4.googleusercontent.com/-cNDZG8GzVA4/VSwRWt6NyBI/AAAAAAAAEGU/y2TJbwUWhfQ/s0/Terminal_001.png)

Available options:

http://zenden.ws/imageuploader-cli

##Building from source

You need Microsoft Visual Studio 2013 to build Image Uploader. 
Installing Visual Studio 2013 Update 4 is strongly recommended (older versions corrupt .rc files after editing)
If you are using a newer MSVC version, you may experience some compilation errors, which can be solved easily (if you fix these errors, please provide a patch or pull request). 
But also you need to recompile 3rd-party 
libraries (boost, glog, jsoncpp, etc) against your compiler version (so I recommend to use Microsoft Visual Studio 2013 Update 4 to avoid all these issues).

You can download ALL necessary libraries and headers (precompiled for MSVC 2013) by following link:

https://bintray.com/artifact/download/zenden/zenden-image-uploader/Image_Uploader_Contrib_MSVC_2013_and_sources_v1.3.2_16.05.2015.7z

mirror https://docs.google.com/file/d/0B_RKTUl10DcrdzFsTF90aS1TUEk/edit?usp=drivesdk

Image Uploader is using the following libraries:

WTL             http://sourceforge.net/projects/wtl/

libcurl         http://curl.haxx.se/libcurl/ (patched version)

openssl         https://www.openssl.org

zlib            http://www.zlib.net

Boost           http://www.boost.org (patched version)

pcre            http://www.pcre.org

pcre++          http://www.daemon.de/PCRE (patched version)

squirrel        http://squirrel-lang.org (patched version)

sqrat		    http://scrat.sourceforge.net (patched version)

ffmpeg          https://www.ffmpeg.org

Boost.Process   https://github.com/zenden2k/boost-process-utf8 (patched version with utf-8 support)

tinyxml         http://sourceforge.net/projects/tinyxml/

gumbo           https://github.com/google/gumbo-parser

gumbo-query     https://github.com/lazytiger/gumbo-query

minizip         http://www.winimage.com/zLibDll/minizip.html

jsoncpp         https://github.com/open-source-parsers/jsoncpp

glog            https://github.com/google/glog

If you want to compile strmbase.lib by yourself you will need:

DirectX 8 SDK

DirectShow BaseClasses
