@echo off
SET CLI_VERSION=0.2.4
set zipcmd="C:/Program Files/7-Zip/7z.exe"

For /F "tokens=2,3 delims= " %%i In (..\Source\VersionInfo.h) Do (set %%i=%%~j)
echo Creating distribution archive for Image Uploader version %_APP_VER% %BUILD%

set temp_dir=portable\temp_%BUILD%_cli_%CLI_VERSION%
set filename=image-uploader-cli-%CLI_VERSION%-cli.7z
echo %temp_dir%

rmdir /q /s  %temp_dir%
del /q output\%filename%
 
mkdir %temp_dir%
mkdir %temp_dir%\Data
mkdir %temp_dir%\Data\Scripts
mkdir %temp_dir%\Data\Update

signtool sign /t http://time.certum.pl /f d:\Backups\ImageUploader\zenden2k.pem  "..\Build\CLI\win32\release\executable\imgupload.exe"  
Copy "..\Build\CLI\win32\release\executable\imgupload.exe" %temp_dir%\imgupload.exe
Copy "..\Build\release\curl-ca-bundle.crt" %temp_dir%\curl-ca-bundle.crt
Copy "..\Data\servers.xml" %temp_dir%\Data\

Copy "..\Data\Scripts\*.nut" %temp_dir%\Data\Scripts\
Copy "..\Data\Update\iu_servers*.xml" %temp_dir%\Data\Update\


cd %temp_dir%
%zipcmd% a -mx9 ..\..\output\image-uploader-cli-%CLI_VERSION%.7z "*"
cd ..\..\

rmdir /q /s  %temp_dir%

