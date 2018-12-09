set /p bintray_key=<d:\Backups\ImageUploader\bintray_key.txt
SET release_version=1.3.2b
For /F "tokens=2,3 delims= " %%i In (..\Source\VersionInfo.h) Do (set %%i=%%~j)
set intaller_name=image-uploader-%IU_APP_VER%-build-%IU_BUILD_NUMBER%-setup.exe
set portable_name=image-uploader-%IU_APP_VER%-build-%IU_BUILD_NUMBER%-portable.7z
set portable_openssl_name=image-uploader-%IU_APP_VER%-build-%IU_BUILD_NUMBER%-openssl-portable.7z
set /p ftp_user=<d:\Backups\ImageUploader\zendenws_u.txt
set /p ftp_pass=<d:\Backups\ImageUploader\zendenws_p.txt