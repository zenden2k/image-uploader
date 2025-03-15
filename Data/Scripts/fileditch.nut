function UploadFile(FileName, options) {
    local name = ExtractFileName(FileName);
    local mime = GetFileMimeType(name);
    nm.setUrl("https://up1.fileditch.com/upload.php");
    nm.addQueryParamFile("files[]", FileName, name, mime);
    nm.doUploadMultipartData();

    if (nm.responseCode() == 200) {
        local sJSON = nm.responseBody();
        local t = ParseJSON(sJSON);
        if (t != null) {
            if ("success" in t && t.success == true) {
                options.setViewUrl(t.files[0].url);
                return 1;
            } 
        }

        WriteLog("error", "[fileditch.com] Cannot obtain file URL from server's response.");
    } else {
        WriteLog("error", "[fileditch.com] Upload failed. Response code: " + nm.responseCode());
    }

    return 0;
}