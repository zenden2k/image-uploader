call set_vars.bat

curl -T output\%portable_name% -uzenden:%bintray_key% -H "X-Bintray-Package:zenden-image-uploader" -H "X-Bintray-Version:%release_version%" https://api.bintray.com/content/zenden/zenden-image-uploader/%portable_name%;publish=1
curl -T output\%intaller_name% -uzenden:%bintray_key% -H "X-Bintray-Package:zenden-image-uploader" -H "X-Bintray-Version:%release_version%" https://api.bintray.com/content/zenden/zenden-image-uploader/%intaller_name%;publish=1
curl -T output\%portable_openssl_name% -uzenden:%bintray_key% -H "X-Bintray-Package:zenden-image-uploader" -H "X-Bintray-Version:%release_version%" https://api.bintray.com/content/zenden/zenden-image-uploader/%portable_openssl_name%;publish=1

echo https://bintray.com/artifact/download/zenden/zenden-image-uploader/%intaller_name% > image-uploader-latest-beta.txt
echo https://bintray.com/artifact/download/zenden/zenden-image-uploader/%portable_name% > image-uploader-latest-beta-portable.txt
echo https://bintray.com/artifact/download/zenden/zenden-image-uploader/%portable_openssl_name% > image-uploader-latest-beta-openssl-portable.txt

curl --upload-file image-uploader-latest-beta.txt  ftp://%ftp_user%:%ftp_pass%@zenden2k.atwebpages.com/zenden2k.atwebpages.com/downloads/
curl --upload-file image-uploader-latest-beta-portable.txt  ftp://%ftp_user%:%ftp_pass%@zenden2k.atwebpages.com/zenden2k.atwebpages.com/downloads/
curl --upload-file image-uploader-latest-beta-openssl-portable.txt  ftp://%ftp_user%:%ftp_pass%@zenden2k.atwebpages.com/zenden2k.atwebpages.com/downloads/

del image-uploader-latest-beta.txt image-uploader-latest-beta-portable.txt image-uploader-latest-beta-openssl-portable.txt
rem curl -v -X POST -uzenden:%bintray_key% -H "X-Bintray-Package:zenden-image-uploader" -H "X-Bintray-Version:%release_version%"  -d '{}'  https://api.bintray.com/content/zenden/zenden-image-uploader/%release_version%/publish 