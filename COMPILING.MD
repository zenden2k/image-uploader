# Building Image Uploader on Windows


To build Image Uploader from source files you will need:
- Git 		https://git-scm.com/downloads
- Microsoft Visual Studio 2019 or newer (with C++ compiler)   https://visualstudio.microsoft.com/downloads/
- CMake (tested with v3.24.0)    https://cmake.org/
- Python 3    https://www.python.org/downloads/
- Conan v1.x (C++ package manager; tested with v1.62)   https://conan.io/

Ensure that your %PATH% environment 
variable includes the directory of your Python and CMake distribution. 
I recommend to install Conan using `pip` utility. 
You can install specific version of conan: `pip3 install conan==1.62`. 

To generate a Visual Studio solution from CMakeLists.txt you need to create a directory for build files and call `cmake`:

```
mkdir Build
cd Build
```

The recommended way is to pass conan profile to the `cmake` command.

```
cmake <path_to_imageuploader_source>\Source -G "Visual Studio 17 2022" -A Win32 -DIU_HOST_PROFILE=default -DIU_BUILD_PROFILE=default 
```
The old way:

```
cmake <path_to_imageuploader_source>\Source -G "Visual Studio 17 2022" -A Win32 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES:STRING=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
```

If you are cross-compiling (from Windows x64 to x86) and want to build FFmpeg libraries from Conan repository then you should use two profiles (`-DIU_HOST_PROFILE=windows_vs2019_x86_release -DIU_BUILD_PROFILE=windows_vs2019_x64_release`)

After that, you can open the generated project in Visual Studio and build it there. Also you can run the command:
```
cmake --build . --config Release
```

The same you can achieve using `cmake-gui` utility.
In cmake-gui you can see all compilation options and easily change them.

If you want to build Image Uploader with separate FFmpeg libraries (`IU_ENABLE_FFMPEG=On` and `IU_FFMPEG_STANDALONE=On`), after building put them (avcodec.lib, avformat.lib, avutil.lib, swscale.lib) into `Contrib\Lib` directory and header files into `Contrib\Include` directory.

If you want to build IU with Microsoft Edge Webview2 support (`IU_ENABLE_WEBVIEW2=On`), you should install package "Microsoft.Web.WebView2" using NuGet package manager (https://docs.microsoft.com/en-us/microsoft-edge/webview2/get-started/win32),
then copy header files `(*.h)` to the "Contrib\Include" directory, and copy `*.lib` files to Contrib\Lib directory.

Some examples, nevermind:

```
cmake ..\Source -G "Visual Studio 16 2019" -A Win32 -DCMAKE_BUILD_TYPE=Debug -DIU_HOST_PROFILE=vs2019_x86_debug -DIU_BUILD_PROFILE=vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On 

cmake ..\Source -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Release -DIU_HOST_PROFILE=../Conan/profiles/vs2019_arm64_release -DIU_BUILD_PROFILE=../Conan/profiles/vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On -DIU_FFMPEG_STANDALONE=On -DIU_ENABLE_MEDIAINFO=Off -DCMAKE_CONFIGURATION_TYPES:STRING=Release 
```

For ARM64:
```
cmake ..\Source -G "Visual Studio 16 2019" -A ARM64 -DCMAKE_BUILD_TYPE=Release -DIU_HOST_PROFILE=../Conan/profiles/vs2019_arm64_release -DIU_BUILD_PROFILE=../Conan/Profiles/vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On -DIU_FFMPEG_STANDALONE=On -DIU_ENABLE_MEDIAINFO=Off
```

# Building on Ubuntu (CLI and Qt GUI)

Use the same approach with Conan to build imgupload on Linux.
As the project is using cmake_find_package_multi generator, we have to use CMake generators with multi-config support. Such generator is `ninja`. 


Before compiling IU install the following packages:

```
sudo apt-get install g++ cmake ninja-build python3-pip
pip3 install conan
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

# Build packages for all platforms at once

Run on Windows:

```
git config --global core.autocrlf false
```

Allow creating symlinks (you can do it by turning on Developer Mode on Windows 11). Another ways is to run Visual Studio Developer Command Prompt as Administrator.


## Prerequisites  (on Windows)

 * msbuild that is a part of Microsoft Visual Studio 2019 or newer (with x86/x64/ARM64 C++ compiler)
 * ATL library for x86/x64/ARM64 platform (a part of Visual Studio)
 * python 3
 * git
 * cmake
 * conan
 * nuget
 * 7-Zip (7z.exe)
 * Inno Setup 6 (iscc.exe)
 * WSL 2 & Ubuntu 20.04+

## Prerequisites On WSL (Ubuntu Linux)

* git 
* conan 
* cmake (version >= 3.17)
* doxygen
* msgfmt (gettext)
* Qt 5 development libraries
* autoconf, automake, libtool, fakeroot for building Debian packages.

You can install most of them by running the command 
```
sudo apt-get install git g++ cmake ninja-build python3-pip doxygen gettext  autoconf automake libtool fakeroot qt5-default
pip3 install conan==1.62 
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

tinyxml         http://sourceforge.net/projects/tinyxml/

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

DirectShow BaseClasses (strmbasd.lib, strmbase.lib) - part of Windows SDK

Libraries for building Mega.nz:

Mega SDK        https://github.com/meganz/sdk

Crypto++        https://www.cryptopp.com/

c-ares          https://c-ares.haxx.se/

libuv           https://github.com/libuv/libuv

Libraries for building tests:

Google Mock     https://github.com/abseil/googletest
