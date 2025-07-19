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
		aarch64) Architecture="arm64"                  ;;
	* ) echo    "Your Architecture '$Architecture' -> ITS NOT SUPPORTED."   
	exit
	;;
	esac
	echo "Detected Architecture : $Architecture"
fi
mkdir ../../Build/Linux/

temp_dir=$(mktemp -d)
echo "Temp directory: ${temp_dir}"
root_dir="${temp_dir}/root"
mkdir -p "${root_dir}"
mkdir -p "${root_dir}/usr/bin"
mkdir -p "${root_dir}/usr/share/uptooda-cli/"
mkdir -p "${root_dir}/usr/share/uptooda-cli/Scripts/"
#rm ./uptooda-cli/usr/share/uptooda-cli/Scripts/*
#mkdir -p ./uptooda-cli/usr/share/uptooda-cli/Update/
#set -e
#objcopy --strip-debug --strip-unneeded ../../Bld-Linux-${Architecture}/CLI/CLI "${root_dir}/usr/bin/uptooda-cli"
$ObjCopy --strip-debug --strip-unneeded ../../Build/CLI/Release/uptooda-cli "${root_dir}/usr/bin/uptooda-cli"
cp ../../Data/servers.xml "${root_dir}/usr/share/uptooda-cli/servers.xml"
cp ../../Data/.env "${root_dir}/usr/share/uptooda-cli/.env"
cp ../../Data/Scripts/*.nut "${root_dir}/usr/share/uptooda-cli/Scripts/"
#cp ../../Data/Update/iu_serversinfo.xml ./uptooda-cli/usr/share/uptooda-cli/Update/iu_serversinfo.xml

if [ ! -f "${root_dir}/usr/bin/uptooda-cli" ]; then
    echo "Executable not found!"
    exit
fi

chmod -R 0755 "${root_dir}/"
chmod -x "${root_dir}/usr/share/uptooda-cli/Scripts/"*.nut
chmod -x "${root_dir}/usr/share/uptooda-cli/servers.xml"

tar_archive="${temp_dir}/uptooda-cli-${AppVersion}-${Architecture}.tar.xz"
cd "${root_dir}" && tar -cJf "${tar_archive}" * && cd -
cp "${tar_archive}" "../../Build/Linux/"

mkdir -p "${root_dir}/DEBIAN"
cp control_ "${root_dir}/DEBIAN/control"
cp dirs "${root_dir}/DEBIAN/dirs"
sed -i -e "s/YOUR_ARCHITECTURE/$Architecture/g" -e "s/IU_APP_VER_CLEAN/${IU_APP_VER_CLEAN}/g" -e "s/IU_BUILD_NUMBER/${IU_BUILD_NUMBER}/g" "${root_dir}/DEBIAN/control"
fakeroot dpkg-deb --build "${root_dir}/" "../../Build/Linux/uptooda-cli_${AppVersion}_${Architecture}.deb"
rm -rf "${temp_dir}"