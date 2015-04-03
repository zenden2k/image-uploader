set /p bintray_key=<d:\Backups\ImageUploader\bintray_key.txt
SET release_version=1.3.1
For /F "tokens=2,3 delims= " %%i In (..\Source\VersionInfo.h) Do (set %%i=%%~j)
curl -T output\image-uploader-%_APP_VER%-build-%BUILD%-portable.7z -uzenden:%bintray_key% -H "X-Bintray-Package:zenden-image-uploader" -H "X-Bintray-Version:%release_version%" https://api.bintray.com/content/zenden/zenden-image-uploader/image-uploader-%_APP_VER%-build-%BUILD%-portable.7z
curl -T output\image-uploader-%_APP_VER%-build-%BUILD%-setup.exe -uzenden:%bintray_key% -H "X-Bintray-Package:zenden-image-uploader" -H "X-Bintray-Version:%release_version%" https://api.bintray.com/content/zenden/zenden-image-uploader/image-uploader-%_APP_VER%-build-%BUILD%-setup.exe
curl -X POST -uzenden:%bintray_key%  https://api.bintray.com/content/zenden/zenden-image-uploader/%release_version%/publish