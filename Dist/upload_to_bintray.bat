set /p bintray_key=<d:\Backups\ImageUploader\bintray_key.txt
SET release_version=1.3.2
For /F "tokens=2,3 delims= " %%i In (..\Source\VersionInfo.h) Do (set %%i=%%~j)
set intaller_name=image-uploader-%_APP_VER%-build-%BUILD%-setup.exe
set portable_name=image-uploader-%_APP_VER%-build-%BUILD%-portable.7z
curl -T output\%portable_name% -uzenden:%bintray_key% -H "X-Bintray-Package:zenden-image-uploader" -H "X-Bintray-Version:%release_version%" https://api.bintray.com/content/zenden/zenden-image-uploader/%portable_name%;publish=1
curl -T output\%intaller_name% -uzenden:%bintray_key% -H "X-Bintray-Package:zenden-image-uploader" -H "X-Bintray-Version:%release_version%" https://api.bintray.com/content/zenden/zenden-image-uploader/%intaller_name%;publish=1
set /p ftp_user=<d:\Backups\ImageUploader\zendenws_u.txt
set /p ftp_pass=<d:\Backups\ImageUploader\zendenws_p.txt
echo https://bintray.com/artifact/download/zenden/zenden-image-uploader/%intaller_name% > image-uploader-latest-beta.txt
echo https://bintray.com/artifact/download/zenden/zenden-image-uploader/%portable_name% > image-uploader-latest-beta-portable.txt

curl --upload-file image-uploader-latest-beta.txt --upload-file image-uploader-latest-beta-portable.txt  ftp://%ftp_user%:%ftp_pass%@zenden.ws/zenden.ws/downloads/

del image-uploader-latest-beta.txt image-uploader-latest-beta-portable.txt
rem curl -v -X POST -uzenden:%bintray_key% -H "X-Bintray-Package:zenden-image-uploader" -H "X-Bintray-Version:%release_version%"  -d '{}'  https://api.bintray.com/content/zenden/zenden-image-uploader/%release_version%/publish 