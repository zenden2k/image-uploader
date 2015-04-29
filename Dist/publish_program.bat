call set_vars.bat
call create_portable.bat
pushd UpdateGen\
php -f gen-update.php core
pushd iu_core
for %%f in (iu_core_*.zip) do (
 curl -T %%f -uzenden:%bintray_key% -H X-Bintray-Package:zenden-image-uploader -H X-Bintray-Version:%release_version% https://api.bintray.com/content/zenden/zenden-image-uploader/%%f;publish=1"

)
rem forfiles /s /m iu_core_*.zip /c "cmd /c echo curl -T @file -uzenden:%bintray_key% -H X-Bintray-Package:zenden-image-uploader -H X-Bintray-Version:%release_version% https://api.bintray.com/content/zenden/zenden-image-uploader/@file;publish=1"
rename iu_core.xml iu_core_beta.xml
curl --upload-file iu_core_beta.xml ftp://%ftp_user%:%ftp_pass%@zenden.ws/zenden.ws/updates/
popd
popd

call create_portable.bat
call create_installer.bat
call upload_to_bintray.bat

