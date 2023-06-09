#!/bin/bash

source <(\
    sed -r '{
                s/#pragma once//g
                s/\r//g
                s/#define ([A-Za-z0-9_]+) "(.*)"/\1="\2"/
            }' <(cat ../../Source/versioninfo.h)
)
AppVersion="${IU_APP_VER_CLEAN}.${IU_BUILD_NUMBER}"
echo ${AppVersion}

Architecture=$1

if [ -z "$Architecture" ]
then
      # Get the machine Architecture
	Architecture=$(uname -m)
	case "$Architecture" in
	    x86)    Architecture="i386"                  ;;
	    ia64)   Architecture="ia64"                 ;;
	    i?86)   Architecture="i386"                  ;;
	    amd64)  Architecture="amd64"                    ;;
	    x86_64) Architecture="amd64"                   ;;
	    sparc64)    Architecture="sparc64"                  ;;
	* ) echo    "Your Architecture '$Architecture' -> ITS NOT SUPPORTED."   
	exit
	;;
	esac
	echo "Detected Architecture : $Architecture"
fi
mkdir ../../Build/Debian/
rm -rf ~/imgupload/
mkdir -p ~/imgupload/DEBIAN
cp control_ ~/imgupload/DEBIAN/control
cp dirs ~/imgupload/DEBIAN/dirs
mkdir -p ~/imgupload/usr/bin
sed -i -e "s/YOUR_ARCHITECTURE/$Architecture/g" -e "s/IU_APP_VER_CLEAN/${IU_APP_VER_CLEAN}/g" -e "s/IU_BUILD_NUMBER/${IU_BUILD_NUMBER}/g" ~/imgupload/DEBIAN/control
mkdir -p ~/imgupload/usr/share/imgupload/
mkdir -p ~/imgupload/usr/share/imgupload/Scripts/
#rm ./imgupload/usr/share/imgupload/Scripts/*
#mkdir -p ./imgupload/usr/share/imgupload/Update/
#set -e
#objcopy --strip-debug --strip-unneeded ../../Bld-Linux-${Architecture}/CLI/CLI ~/imgupload/usr/bin/imgupload
objcopy --strip-debug --strip-unneeded ../../Build/CLI/Release/CLI ~/imgupload/usr/bin/imgupload
cp ../../Data/servers.xml ~/imgupload/usr/share/imgupload/servers.xml
cp ../../Data/Scripts/*.nut ~/imgupload/usr/share/imgupload/Scripts/
#cp ../../Data/Update/iu_serversinfo.xml ./imgupload/usr/share/imgupload/Update/iu_serversinfo.xml

if [ ! -f ~/imgupload/usr/bin/imgupload ]; then
    echo "Executable not found!"
    exit
fi

chmod -R 0755 ~/imgupload/
chmod -x ~/imgupload/usr/share/imgupload/Scripts/*.nut
chmod -x ~/imgupload/usr/share/imgupload/servers.xml

fakeroot dpkg-deb --build ~/imgupload/ "../../Build/Debian/imgupload_${AppVersion}_${Architecture}.deb"
