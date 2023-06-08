# Building Image Uploader GUI (or CLI) on Windows

Before building Image Uploader, install Python 3 interpreter for performing pre-build and post-build events. Ensure that your %PATH% environment 
variable includes the directory of your Python distribution.

To build Image Uploader from source files you will need:
- Git 		https://git-scm.com/downloads
- Microsoft Visual Studio 2019 or newer (with C++ compiler)   https://visualstudio.microsoft.com/downloads/
- CMake (tested with v3.24.0)    https://cmake.org/
- Python 3    https://www.python.org/downloads/
- Conan (C++ package manager)   https://conan.io/

I recommend to install Conan using `pip` utility.

To generate a Visual Studio solution from CMakeLists.txt you need to create a directory for 
temporary files and call cmake:

```
mkdir Build
cd Build
```

```
cmake <path_to_imageuploader_source>\Source -G "Visual Studio 17 2022" -A Win32 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES:STRING=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
```

Another way is to pass conan profile name to cmake:

```
cmake <path_to_imageuploader_source>\Source -G "Visual Studio 17 2022" -DIU_HOST_PROFILE=default
```

Then open solution in Visual Studio **or** run command:
```
cmake --build . --config Release
```

```
cmake ..\Source -G "Visual Studio 16 2019" -A Win32 -DCMAKE_BUILD_TYPE=Debug -DIU_HOST_PROFILE=vs2019_x86_debug -DIU_BUILD_PROFILE=vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On 

cmake ..\Source -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Release -DIU_HOST_PROFILE=../Conan/profiles/vs2019_arm64_release -DIU_BUILD_PROFILE=../Conan/profiles/vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On -DIU_FFMPEG_STANDALONE=On -DIU_ENABLE_MEDIAINFO=Off -DCMAKE_CONFIGURATION_TYPES:STRING=Release 
```

For ARM64:
```
cmake ..\Source -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Release -DIU_HOST_PROFILE=../Conan/profiles/vs2019_arm64_release -DIU_BUILD_PROFILE=../Conan/Profiles/vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On -DIU_FFMPEG_STANDALONE=On -DIU_ENABLE_MEDIAINFO=Off
```
The more convenient way to achieve the same is to use `cmake-gui`.
In cmake-gui you can see all compiling options and change them easily.

If you want to build Image Uploader with FFmpeg libraries (`IU_ENABLE_FFMPEG=On`), you have to built them separately and put them (avcodec.lib, avformat.lib, avutil.lib, swscale.lib) into `Contrib\Lib` directory.

If you need Microsoft Edge Webview2 (`IU_ENABLE_WEBVIEW2=On`), you should install it using NuGet package manager (https://docs.microsoft.com/en-us/microsoft-edge/webview2/get-started/win32),
then copy header files `(*.h)` to the "Contrib\Include" directory, and copy `*.lib` files to Contrib\Lib directory.

# Building on Ubuntu (CLI and Qt GUI)

Use the same approach with Conan to build imgupload on Linux.
As the project is using cmake_find_package_multi generator, we have to use CMake generators with multi-config support. Such generator is `ninja`. 


Before compiling IU install the following packages:

```
pip3 install conan
sudo apt-get install g++ cmake ninja-build 
```

Configure and build the project:

```bash
git clone https://github.com/zenden2k/image-uploader.git
cd image-uploader
mkdir Build
cd Build
cmake ../Source  -G "Ninja Multi-Config" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES:STRING=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
cmake --build . --config Release
```

If you want to compile qimageuploader (GUI) you have to install Qt 5 dev libraries before compiling:
```
sudo apt install qt5-default
```

You may also need these packages for creating a .deb package:
```
sudo apt-get install autoconf automake libtool fakeroot
```

and pass `-DIU_BUILD_QIMAGEUPLOADER=On` option to the `cmake` command.

# Image Uploader is using the following libraries:

WTL             http://sourceforge.net/projects/wtl/

libcurl         http://curl.haxx.se/libcurl/ 

openssl         https://www.openssl.org

zlib            http://www.zlib.net

Boost           http://www.boost.org

pcre            http://www.pcre.org

pcre++          http://www.daemon.de/PCRE (patched version)

squirrel        http://squirrel-lang.org (patched version)

sqrat		    http://scrat.sourceforge.net (patched version)

ffmpeg          https://www.ffmpeg.org

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
