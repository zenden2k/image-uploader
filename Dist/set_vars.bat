set /p bintray_key=<d:\Backups\ImageUploader\bintray_key.txt
SET release_version=1.3.2ab
For /F "tokens=2,3 delims= " %%i In (..\Source\VersionInfo.h) Do (set %%i=%%~j)
set intaller_name=image-uploader-%_APP_VER%-build-%BUILD%-setup.exe
set portable_name=image-uploader-%_APP_VER%-build-%BUILD%-portable.7z
set /p ftp_user=<d:\Backups\ImageUploader\zendenws_u.txt
set /p ftp_pass=<d:\Backups\ImageUploader\zendenws_p.txt