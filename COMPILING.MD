﻿# Building Image Uploader GUI (or CLI) on Windows

Before building Image Uploader you should copy file "Source\versioninfo.h.dist" to "Source\versioninfo.h". This file will be regenerated 
by "versioninc.py" utility and will contain information about build number, version, date, git commit hash.
Python 3 interpreter should be installed for performing pre-build and post-build events. Ensure that your %PATH% environment 
variable includes the directory of your Python distribution.

To build Image Uploader from source files you will need:
- Git 		https://git-scm.com/downloads
- Microsoft Visual Studio 2019 or newer (with C++ compiler)   https://visualstudio.microsoft.com/downloads/
- CMake (tested with v3.18.2)    https://cmake.org/
- Python 3    https://www.python.org/downloads/
- Conan (C++ package manager)   https://conan.io/

To generate a Visual Studio solution from CMakeLists.txt you need to create a directory for 
temporary files and call cmake:

```
mkdir Build
cd Build
cmake <path_to_imageuploader_source>\Source
```

The last command will generate project files for your default compiler (it should be Visual Studio). 
The more convenient way to achieve the same is to use cmake-gui.
In cmake-gui you can see all compiling options and change them easily.

If you want to build Image Uploader with FFmpeg libraries (`IU_ENABLE_FFMPEG=On`), you have to built them separately amd put them (avcodec.lib, avformat.lib, avutil.lib, swscale.lib) into `Contrib\Lib` directory.

If you need Microsoft Edge Webview2 (`IU_ENABLE_WEBVIEW2=On`), you should install it using NuGet package manager (https://docs.microsoft.com/en-us/microsoft-edge/webview2/get-started/win32),
then copy header files `(*.h)` to the "Contrib\Include" directory, and copy `*.lib` files to Contrib\Lib directory.

# Building console utility (imgupload) on Ubuntu (incomplete instruction)

Before compiling install the following packages:

```
sudo apt-get install g++ cmake cmake-qt-gui autoconf automake libtool fakeroot
```

Use the same approach with Conan to build imgupload on Linux.

# Building qimageuploader on Ubuntu

sudo apt install qt5-default libavcodec-dev libavformat-dev libavutil-dev libswscale-dev

# Image Uploader is using the following libraries:

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

libwebp         https://github.com/webmproject/libwebp

base64          https://github.com/aklomp/base64

UTF8-CPP        http://utfcpp.sourceforge.net/ 

DirectShow BaseClasses (strmbasd.lib, strmbase.lib) - part of Windows SDK

Libraries for building Mega.nz:

Mega SDK        https://github.com/meganz/sdk

Crypto++        https://www.cryptopp.com/

c-ares          https://c-ares.haxx.se/

libuv           https://github.com/libuv/libuv

Libraries for building tests:

Google Mock     https://github.com/abseil/googletest