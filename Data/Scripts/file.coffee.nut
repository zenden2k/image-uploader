function UploadFile(FileName, options) {
    local name = ExtractFileName(FileName);
    local mime = GetFileMimeType(name);
    nm.setUrl("https://file.coffee/api/file/upload");
    nm.addQueryParamFile("file", FileName, name, mime);
    nm.doUploadMultipartData();

    if (nm.responseCode() > 0) {
        local sJSON = nm.responseBody();
        local t = ParseJSON(sJSON);
        if (t != null) {
            if (t.success) {
                options.setDirectUrl(t.url);
                return 1;
            } else {
                WriteLog("error", "[file.coffee] " + t.message);
            }
        } 
    }

    return 0;
}