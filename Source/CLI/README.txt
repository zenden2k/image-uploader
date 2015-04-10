Building in Ubuntu:

First you have to compile sqplus and jsoncpp (or download the "Contrib" archive with precompiled libraries)
Put compiled libraries into directory <source_root>/Contrib/Lib/Linux/<architecture>/
libjsoncpp.a
libsqplus.a
libsqstdlib.a
libsquirrel.a

Install dev libraries:

sudo apt-get install qt5-make libcurl4-openssl-dev libpcre3-dev libgoogle-glog-dev  

Create directory:
mkdir <source_root>/Build/CLI/linux/

Then compile:

qmake
make



