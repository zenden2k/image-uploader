AppVersion="0.2.4"

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

cp imgupload/DEBIAN/control_ imgupload/DEBIAN/control
mkdir ./imgupload/usr/bin/
sed -i "s/YOUR_ARCHITECTURE/$Architecture/g" imgupload/DEBIAN/control
mkdir -p ./imgupload/usr/share/imgupload/
mkdir -p ./imgupload/usr/share/imgupload/Scripts/
#rm ./imgupload/usr/share/imgupload/Scripts/*
#mkdir -p ./imgupload/usr/share/imgupload/Update/
#set -e
cp ../../Build/CLI/linux/${Architecture}/release/executable/imgupload ./imgupload/usr/bin/
cp ../../Data/servers.xml ./imgupload/usr/share/imgupload/servers.xml
cp ../../Data/Scripts/*.nut ./imgupload/usr/share/imgupload/Scripts/
#cp ../../Data/Update/iu_serversinfo.xml ./imgupload/usr/share/imgupload/Update/iu_serversinfo.xml

if [ ! -f ./imgupload/usr/bin/imgupload ]; then
    echo "Executable not found!"
    exit
fi

fakeroot dpkg-deb --build imgupload/ "imgupload_${AppVersion}_${Architecture}.deb"