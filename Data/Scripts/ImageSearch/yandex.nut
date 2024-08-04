const CURLOPT_FOLLOWLOCATION = 52;

function SearchByImage(FileName, options) {
    local imageInfo = GetImageInfo(FileName);
    if (imageInfo.Width == 0 || imageInfo.Height == 0) {
        imageInfo.Width = 640;
        imageInfo.Height = 480;
    } 

    nm.setUrl("https://yandex.ru/images/search?rpt=imageview");
    nm.addQueryHeader("Origin", "https://yandex.ru");
    nm.addQueryParamFile("upfile", FileName, ExtractFileName(FileName), "");
    nm.addQueryParam("original_width", imageInfo.Width);
    nm.addQueryParam("original_height", imageInfo.Height);
    nm.addQueryParam("prg", "1");
    nm.setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0); 
    nm.doUploadMultipartData();
    
    if (nm.responseCode() < 301 || nm.responseCode() > 303) {
        WriteLog("error", "Server sent unexpected result.");
        return {
            status = 0,
            message = "Server sent unexpected result."
        }
    }

    local url = nm.responseHeaderByName("Location"); 

    ShellOpenUrl(url);

    return {
        status = 1
    }
}