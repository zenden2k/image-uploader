@echo off
echo Have you called create_portable ?
pause
call set_vars.bat
pushd UpdateGen\
php -f gen-update.php serversinfo
pushd iu_serversinfo
for %%f in (iu_serversinfo_*.dat) do (
curl --upload-file %%f ftp://%ftp_user%:%ftp_pass%@zenden.ws/zenden.ws/updates/
 curl -T %%f -uzenden:%bintray_key% -H X-Bintray-Package:zenden-image-uploader -H X-Bintray-Version:%release_version% https://api.bintray.com/content/zenden/zenden-image-uploader/%%f;publish=1"

)
rem forfiles /s /m iu_core_*.zip /c "cmd /c echo curl -T @file -uzenden:%bintray_key% -H X-Bintray-Package:zenden-image-uploader -H X-Bintray-Version:%release_version% https://api.bintray.com/content/zenden/zenden-image-uploader/@file;publish=1"
rename iu_serversinfo.xml iu_serversinfo_new.xml
curl --upload-file iu_serversinfo_new.xml ftp://%ftp_user%:%ftp_pass%@zenden.ws/zenden.ws/updates/
popd
popd


