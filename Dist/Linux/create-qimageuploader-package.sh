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
rm -rf ~/zenden2k-imageuploader/
mkdir -p ~/zenden2k-imageuploader/DEBIAN
cp control_qimageuploader ~/zenden2k-imageuploader/DEBIAN/control
cp dirs ~/zenden2k-imageuploader/DEBIAN/dirs
mkdir -p ~/zenden2k-imageuploader/usr/share/applications
mkdir -p ~/zenden2k-imageuploader/usr/share/pixmaps
cp ../../Source/res/icon_main_big.png ~/zenden2k-imageuploader/usr/share/pixmaps/zenden2k-imageuploader.png
cp qimageuploader.desktop ~/zenden2k-imageuploader/usr/share/applications/zenden2k-imageuploader.desktop
mkdir -p ~/zenden2k-imageuploader/usr/bin
sed -i "s/YOUR_ARCHITECTURE/$Architecture/g" ~/zenden2k-imageuploader/DEBIAN/control
sed -i -e "s/YOUR_ARCHITECTURE/$Architecture/g" -e "s/IU_APP_VER_CLEAN/${IU_APP_VER_CLEAN}/g" -e "s/IU_BUILD_NUMBER/${IU_BUILD_NUMBER}/g" ~/zenden2k-imageuploader/DEBIAN/control 
mkdir -p ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/
mkdir -p ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Scripts/
mkdir -p ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Favicons/
$ObjCopy --strip-debug --strip-unneeded ../../Build/bin/Release/qimageuploader ~/zenden2k-imageuploader/usr/bin/zenden2k-imageuploader
cp ../../Data/servers.xml ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/servers.xml
cp ../../Data/.env ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/.env
cp ../../Data/Scripts/*.nut ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Scripts/
cp ../../Data/Favicons/*.ico ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Favicons/

if [ ! -f ~/zenden2k-imageuploader/usr/bin/zenden2k-imageuploader ]; then
    echo "Executable not found!"
    exit
fi

chmod -R 0755 ~/zenden2k-imageuploader/
chmod -x ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Scripts/*.nut
chmod -x ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/servers.xml
 
fakeroot dpkg-deb --build ~/zenden2k-imageuploader/ "../../Build/Linux/zenden2k-imageuploader-qt_${AppVersion}_${Architecture}.deb"
