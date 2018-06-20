@echo off
set zipcmd="C:/Program Files/7-Zip/7z.exe"

For /F "tokens=2,3 delims= " %%i In (..\Source\VersionInfo.h) Do (set %%i=%%~j)
echo Creating distribution archive for Image Uploader version %_APP_VER% %BUILD%

set temp_dir=serverchecker\temp
set filename=servers-checker-1.0.1-iu-core-%_APP_VER%-build-%BUILD%.7z
echo %temp_dir%

rmdir /q /s  %temp_dir%
del /q output\%filename%
 
mkdir %temp_dir%
mkdir %temp_dir%\Data
mkdir %temp_dir%\Data\Thumbnails\
mkdir %temp_dir%\Data\Favicons
mkdir %temp_dir%\Data\Scripts
mkdir %temp_dir%\Data\Scripts\Lang
mkdir %temp_dir%\Data\Scripts\Utils
mkdir %temp_dir%\Data\Servers
mkdir %temp_dir%\Data\Update
mkdir %temp_dir%\Data\Utils

signtool sign /t http://time.certum.pl /f d:\Backups\ImageUploader\zenden2k.pem "..\Build\ServerTool\release\ServerListTool.exe"  
Copy "..\Build\ServerTool\release\ServerListTool.exe" %temp_dir%\
Copy "..\Build\ServerTool\release\testfile.jpg" %temp_dir%\
Copy "..\Data\servers.xml" %temp_dir%\Data\
Copy "..\Data\Favicons\*.ico" %temp_dir%\Data\Favicons\
Copy "..\Data\Scripts\*.nut" %temp_dir%\Data\Scripts\
Copy "..\Data\Scripts\Lang\*.json" %temp_dir%\Data\Scripts\Lang\
Copy "..\Data\Scripts\Utils\*.nut" %temp_dir%\Data\Scripts\Utils\
rem signtool sign  /t http://timestamp.digicert.com /f "d:\Backups\ImageUploader\3315593d7023a0aeb48042349dc4fd40.pem" "%temp_dir%\Image Uploader.exe" "%temp_dir%\ExplorerIntegration.dll" "%temp_dir%\ExplorerIntegration64.dll"

cd %temp_dir%
%zipcmd% a -mx9 ..\..\output\%filename% "*"
cd ..\..\

rem rmdir /q /s  %temp_dir%

