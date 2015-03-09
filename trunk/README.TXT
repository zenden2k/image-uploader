
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

---  COMPILING IMAGE UPLOADER ----

You need Microsoft Visual C++ at least 9.0 version (Microsoft Visual Studio 2008 SP1).
Also you have to install Microsoft Windows 7 Platform SDK for enabling Windows 7 related features. 

In order to compile Image Uploader you will need the following libraries:

WTL		http://sourceforge.net/projects/wtl/
libcurl		http://curl.haxx.se/libcurl/
openssl		https://www.openssl.org
zlib		http://www.zlib.net
pcre		http://www.pcre.org
pcre++		http://www.daemon.de/PCRE
squirrel	http://squirrel-lang.org
sqplus		http://sourceforge.net/projects/sqplus/
ffmpeg		https://www.ffmpeg.org
tinyxml		http://sourceforge.net/projects/tinyxml/
zthreads	http://zthread.sourceforge.net
minizip		http://www.winimage.com/zLibDll/minizip.html
jsoncpp		https://github.com/open-source-parsers/jsoncpp

You can download necessary libraries by following link: https://bintray.com/artifact/download/zenden/zenden-image-uploader/Contrib_MSVC_2008_SP1_and_sources.7z
They are precompiled for MSVC 2008 SP1.

If you want to compile strmbase.lib by yourself yow will need:
DirectX 8 SDK
DirectShow BaseClasses

If you receive errors related to "TOKEN_ELEVATION" or "ITaskBar3", it means you haven't installed latest Platform SDK. 
Also you can change WINVER value into "stdafx.h" file to 0x0600 or 0x0501 and compile Image Uploader 
without Windows 7 TaskBar features support.