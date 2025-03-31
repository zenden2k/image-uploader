# Building Image Uploader on Windows

To build Image Uploader from source files you will need:
- Git 		https://git-scm.com/downloads
- Microsoft Visual Studio 2019 or newer (with C++ compiler)   https://visualstudio.microsoft.com/downloads/
- Windows 10 SDK
- ATL library for your platform (a part of Visual Studio)
- CMake (tested with v3.24.0)    https://cmake.org/
- Python 3    https://www.python.org/downloads/
- Conan v2.x (C++ package manager; tested with v2.11.0)   https://conan.io/

Ensure that your %PATH% environment 
variable includes the directory of your Python and CMake distribution. 
I recommend to install Conan using `pip` utility. 
You can install specific version of conan: `pip3 install conan==2.11.0`. 
Run this command to add conan remote:

```bash
conan remote add zenden2k https://conan.svistunov.dev --index 0
```
To generate a Visual Studio solution from CMakeLists.txt you need to create a directory for build files and call `cmake`:

```bash
mkdir Build
cd Build
```

The recommended way is to pass conan profile to the `cmake` command.

```bash
cmake <path_to_imageuploader_source>\Source -G "Visual Studio 17 2022" -A Win32 -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCONAN_HOST_PROFILE=default -DCONAN_BUILD_PROFILE=default 
```

If you are cross-compiling (from Windows x64 to x86) and want to build FFmpeg libraries from Conan repository then you should use two profiles (`-DCONAN_HOST_PROFILE=windows_vs2019_x86_release -DCONAN_BUILD_PROFILE=windows_vs2019_x64_release`)

After that, you can open the generated project in Visual Studio and build it there. Also you can run the command:
```
cmake --build . --config Release
```

The same you can achieve using `cmake-gui` utility.
In cmake-gui you can see all compilation options and easily change them.

If you want to build Image Uploader with separate FFmpeg libraries (`IU_ENABLE_FFMPEG=On` and `IU_FFMPEG_STANDALONE=On`), after building put them (avcodec.lib, avformat.lib, avutil.lib, swscale.lib) into `Contrib\Lib` directory and header files into `Contrib\Include` directory.

Some examples, nevermind:

```bash
cmake ..\Source -G "Visual Studio 16 2019" -A Win32 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCONAN_HOST_PROFILE=vs2019_x86_debug -DCONAN_BUILD_PROFILE=vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On 

cmake ..\Source -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCONAN_HOST_PROFILE=../Conan/profiles/vs2019_arm64_release -DCONAN_BUILD_PROFILE=../Conan/profiles/vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On -DIU_FFMPEG_STANDALONE=On -DIU_ENABLE_MEDIAINFO=Off -DCMAKE_CONFIGURATION_TYPES:STRING=Release 
```

For ARM64:
```bash
cmake ..\Source -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCONAN_HOST_PROFILE=../Conan/profiles/vs2019_arm64_release -DCONAN_BUILD_PROFILE=../Conan/Profiles/vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On -DIU_FFMPEG_STANDALONE=On -DIU_ENABLE_MEDIAINFO=Off
```

## Running Tests in Visual Studio IDE

Open *Tools* -> *Options* -> *Test Adapter for Google Test* -> *General*

Set *Additional test execution parameters*:

```
--gtest_filter=-NetworkClientTest.*
```

Save settings, then open *Test* -> *Test explorer* and run all the tests.

# Building on Ubuntu (CLI and Qt GUI)

Use the same approach with Conan to build imgupload on Ubuntu 20.04.

Before compiling IU install the following packages:

```
sudo apt-get install g++ git ninja-build python3-pip # cmake
pip3 install conan
conan remote add zenden2k https://conan.svistunov.dev --index 0
```

Note that Ubuntu 20.04 has CMake v3.16 but the project requires >= v3.17.
You can install the latest binary distribution from the official website https://cmake.org/

Configure and build the project:

```bash
git clone https://github.com/zenden2k/image-uploader.git
cd image-uploader
mkdir Build
cd Build
cmake ../Source  -G "Ninja Multi-Config" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCMAKE_CONFIGURATION_TYPES:STRING=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
cmake --build . --config Release
```

If you want to compile qimageuploader (GUI) you have to install Qt 5 dev libraries before compiling:
```bash
sudo apt install qt5-default
```

You may also need these packages for creating a .deb package:
```bash
sudo apt-get install autoconf automake libtool fakeroot
```

and pass `-DIU_BUILD_QIMAGEUPLOADER=On` option to the `cmake` command.

# Build packages for all platforms at once

To build the release I use a combination of Windows and WSL 2 (Ubuntu 20.04).

Run on Windows:

```bash
git config --global core.autocrlf false
conan remote add zenden2k https://conan.svistunov.dev --index 0
```

Allow creating symlinks (you can do it by turning on Developer Mode on Windows 11). Another way is to run Visual Studio Developer Command Prompt as Administrator.

## Prerequisites  (on Windows)

 * msbuild that is a part of Microsoft Visual Studio 2019 or newer (with x86/x64/ARM64 C++ compiler)
 * Windows 10 SDK
 * ATL library for x86/x64/ARM64 platform (a part of Visual Studio)
 * python 3
 * git
 * cmake
 * conan (version >= 2.0)
 * nuget?
 * 7-Zip (7z.exe)
 * Inno Setup 6 (iscc.exe)
 * WSL 2 & Ubuntu 20.04+

Add Conan remote:

```bash
conan remote add zenden2k https://conan.svistunov.dev --index 0
```

## Prerequisites On WSL (Ubuntu Linux)

* git 
* conan (version >= 2.0)
* cmake (version >= 3.17)
* doxygen
* msgfmt (gettext)
* Qt 5 development libraries
* autoconf, automake, libtool, fakeroot for building Debian packages.

You can install most of them by running the command 
```bash
sudo apt-get install git g++ cmake ninja-build python3-pip doxygen gettext  autoconf automake libtool fakeroot qt5-default
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu # for cross-compiling
pip3 install conan==2.11.0
```
Note that Ubuntu 20.04 has CMake v3.16 but the script requires >= v3.17.
You can install the latest binary distribution from the official website https://cmake.org/

All programs must be added to the PATH environment variable.
Copy `versioninfo.h` to the `Dist` directory. Edit this file and set the values ​​you need. The build number will be incremented automatically.
 
Run `python create_build.py` from Visual Studio Developer Command Prompt.

# Image Uploader is using the following libraries:

WTL             http://sourceforge.net/projects/wtl/

libcurl         http://curl.haxx.se/libcurl/ 

openssl         https://www.openssl.org

zlib            http://www.zlib.net

Boost           http://www.boost.org

Qt              https://www.qt.io/

pcre            http://www.pcre.org

pcre++          http://www.daemon.de/PCRE (patched version)

squirrel        http://squirrel-lang.org (patched version)

sqrat		    http://scrat.sourceforge.net (patched version)

ffmpeg          https://www.ffmpeg.org

tinyxml2        https://github.com/leethomason/tinyxml2

gumbo           https://github.com/google/gumbo-parser

gumbo-query     https://github.com/lazytiger/gumbo-query

minizip         http://www.winimage.com/zLibDll/minizip.html

jsoncpp         https://github.com/open-source-parsers/jsoncpp

glog            https://github.com/google/glog

libwebp         https://github.com/webmproject/libwebp

base64          https://github.com/aklomp/base64

UTF8-CPP        http://utfcpp.sourceforge.net/ 

libheif         https://github.com/strukturag/libheif 

WinToast        https://github.com/mohabouje/WinToast

argparse        https://github.com/p-ranav/argparse

xdgmime         https://gitlab.freedesktop.org/xdg/xdgmime

DirectShow BaseClasses (strmbasd.lib, strmbase.lib) - part of Windows SDK

**Libraries for building Mega.nz:**

Mega SDK        https://github.com/meganz/sdk

Crypto++        https://www.cryptopp.com/

c-ares          https://c-ares.haxx.se/

libuv           https://github.com/libuv/libuv

**Libraries for building tests:**

Google Mock     https://github.com/abseil/googletest
