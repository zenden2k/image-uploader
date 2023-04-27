AppVersion="0.1.2"

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

mkdir -p ~/zenden2k-imageuploader/DEBIAN
cp control_qimageuploader ~/zenden2k-imageuploader/DEBIAN/control
cp dirs ~/zenden2k-imageuploader/DEBIAN/dirs
mkdir -p ~/zenden2k-imageuploader/usr/bin
sed -i "s/YOUR_ARCHITECTURE/$Architecture/g" ~/zenden2k-imageuploader/DEBIAN/control
mkdir -p ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/
mkdir -p ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Scripts/
mkdir -p ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Favicons/
objcopy --strip-debug --strip-unneeded ../../Build/bin/Release/qimageuploader ~/zenden2k-imageuploader/usr/bin/zenden2k-imageuploader
cp ../../Data/servers.xml ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/servers.xml
cp ../../Data/Scripts/*.nut ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Scripts/
cp ../../Data/Favicons/*.ico ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Favicons/

if [ ! -f ~/zenden2k-imageuploader/usr/bin/zenden2k-imageuploader ]; then
    echo "Executable not found!"
    exit
fi

chmod -R 0755 ~/zenden2k-imageuploader/
chmod -x ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/Scripts/*.nut
chmod -x ~/zenden2k-imageuploader/usr/share/zenden2k-imageuploader/servers.xml

fakeroot dpkg-deb --build ~/zenden2k-imageuploader/ "zenden2k-imageuploader-qt_${AppVersion}_${Architecture}.deb"
