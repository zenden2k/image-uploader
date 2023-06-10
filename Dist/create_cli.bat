@echo off
SET CLI_VERSION=1.4.0
set zipcmd="C:/Program Files/7-Zip/7z.exe"

For /F "tokens=2,3 delims= " %%i In (..\Source\VersionInfo.h) Do (set %%i=%%~j)
echo Creating distribution archive for Image Uploader version %IU_APP_VER% %IU_BUILD_NUMBER%

set temp_dir=..\Build\portable\temp_%BUILD%_cli_%CLI_VERSION%
set filename=imgupload-%IU_APP_VER%-build-%IU_BUILD_NUMBER%-cli.7z
echo %temp_dir%

rmdir /q /s  %temp_dir%
del /q output\%filename%
 
mkdir %temp_dir%
mkdir %temp_dir%\Data
mkdir %temp_dir%\Data\Scripts
mkdir %temp_dir%\Data\Update

rem signtool sign /t http://time.certum.pl /f d:\Backups\ImageUploader\zenden2k.pem  "..\Build\CLI\win32\release\executable\imgupload.exe"  
Copy "..\Build\CLI\Release\CLI.exe" %temp_dir%\imgupload.exe
Copy "curl-ca-bundle.crt" %temp_dir%\curl-ca-bundle.crt
Copy "..\Data\servers.xml" %temp_dir%\Data\

Copy "..\Data\Scripts\*.nut" %temp_dir%\Data\Scripts\
Copy "..\Data\Update\iu_servers*.xml" %temp_dir%\Data\Update\


cd %temp_dir%
%zipcmd% a -mx9 ..\..\output\%filename% "*"
cd ..\..\

rmdir /q /s  %temp_dir%

