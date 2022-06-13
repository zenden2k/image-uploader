function UploadFile(FileName, options) {
    local name = ExtractFileName(FileName);
    local mime = GetFileMimeType(name);
    nm.setUrl("https://geekpic.net/client.php");
    nm.addQueryHeader("MIME-Version", "1.0");
    nm.addQueryHeader("Accept-Language", "ru-RU,en,*");
    nm.setUserAgent("Mozilla/5.0 (Windows NT 6.3) Qt/4.8.6 Screenpic/0.14.3");
    nm.addQueryParam("image", Base64Encode(GetFileContents(FileName)));
    nm.addQueryParam("name", name);
    nm.addQueryParam("description", name);
    nm.doUploadMultipartData();
    if (nm.responseCode() == 200) {
        local sJSON = nm.responseBody();
        local t = ParseJSON(sJSON);
        if (t != null && t.success) {
            options.setViewUrl(t.preview);
            options.setThumbUrl(t.thumbs.small);
            options.setDirectUrl(t.link);
            return 1;
        }
    }
    return 0;
}