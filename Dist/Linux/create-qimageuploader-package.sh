#!/bin/bash

source <(\
    sed -r '{
                s/#pragma once//g
                s/\r//g
                s/#define ([A-Za-z0-9_]+) "(.*)"/\1="\2"/
            }' <(cat ../../Source/versioninfo.h)
)
AppVersion="${IU_APP_VER_CLEAN}.${IU_BUILD_NUMBER}"
Architecture=$1
ObjCopy=$2
if [ -z "$ObjCopy" ]
then
	ObjCopy=objcopy
fi

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
fi


echo "Detected Architecture : $Architecture"
mkdir ../../Build/Linux/
rm -rf ~/uptooda/
mkdir -p ~/uptooda/DEBIAN
cp control_qimageuploader ~/uptooda/DEBIAN/control
cp dirs ~/uptooda/DEBIAN/dirs
mkdir -p ~/uptooda/usr/share/applications
mkdir -p ~/uptooda/usr/share/pixmaps
cp ../../Source/res/icon_main_big.png ~/uptooda/usr/share/pixmaps/uptooda.png
cp qimageuploader.desktop ~/uptooda/usr/share/applications/uptooda.desktop
mkdir -p ~/uptooda/usr/bin
sed -i "s/YOUR_ARCHITECTURE/$Architecture/g" ~/uptooda/DEBIAN/control
sed -i -e "s/YOUR_ARCHITECTURE/$Architecture/g" -e "s/IU_APP_VER_CLEAN/${IU_APP_VER_CLEAN}/g" -e "s/IU_BUILD_NUMBER/${IU_BUILD_NUMBER}/g" ~/uptooda/DEBIAN/control 
mkdir -p ~/uptooda/usr/share/uptooda/
mkdir -p ~/uptooda/usr/share/uptooda/Scripts/
mkdir -p ~/uptooda/usr/share/uptooda/Favicons/
$ObjCopy --strip-debug --strip-unneeded ../../Build/bin/Release/qimageuploader ~/uptooda/usr/bin/uptooda
cp ../../Data/servers.xml ~/uptooda/usr/share/uptooda/servers.xml
cp ../../Data/.env ~/uptooda/usr/share/uptooda/.env
cp ../../Data/Scripts/*.nut ~/uptooda/usr/share/uptooda/Scripts/
cp ../../Data/Favicons/*.ico ~/uptooda/usr/share/uptooda/Favicons/

if [ ! -f ~/uptooda/usr/bin/uptooda ]; then
    echo "Executable not found!"
    exit
fi

chmod -R 0755 ~/uptooda/
chmod -x ~/uptooda/usr/share/uptooda/Scripts/*.nut
chmod -x ~/uptooda/usr/share/uptooda/servers.xml
 
fakeroot dpkg-deb --build ~/uptooda/ "../../Build/Linux/uptooda_${AppVersion}_${Architecture}.deb"
