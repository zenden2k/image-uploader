Building in Ubuntu:

Install g++.
sudo apt-get install  g++

First you have to compile sqplus and jsoncpp (or download the "Contrib" archive with precompiled libraries)

For compiling jsoncpp:
cd jsoncpp-master
sudo apt-get install cmake-gui
cmake-gui
Choose source and build directory and click the "Configure"
 (if not configured yet) and "Generate" button. After that go to the build directory and run "make".

For compiling sqplus:
32 bit:
cd SQUIRREL2_1_1_sqplus_snapshot_20080713
make sq32
cd sqplus
make sqplus

64 bit:
cd SQUIRREL2_1_1_sqplus_snapshot_20080713
make sq64
cd sqplus
make sqplus64 

Put compiled libraries into directory <source_root>/Contrib/Lib/Linux/<architecture>/
libjsoncpp.a
libsqplus.a
libsqstdlib.a
libsquirrel.a

Install dev libraries:

sudo apt-get install qt5-qmake libcurl4-openssl-dev libpcre3-dev libgoogle-glog-dev libboost-system-dev libboost-filesystem-dev

Create directory:
mkdir <source_root>/Build/CLI/linux/

Then compile:

qmake
make



