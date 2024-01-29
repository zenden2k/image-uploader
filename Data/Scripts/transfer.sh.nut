function UploadFile(FileName, options) {
    local name = ExtractFileName(FileName);
    local mime = GetFileMimeType(name);
    nm.setUrl("https://transfer.sh/" + nm.urlEncode(name));
    nm.setMethod("PUT");
    nm.doUpload(FileName, "");

    if (nm.responseCode() == 200 || nm.responseCode() == 201) {
        options.setDirectUrl(nm.responseBody());
        // This link not supposed to be opened in web browser
        //options.setDeleteUrl(nm.responseHeaderByName("x-url-delete"));    
        return 1; 
    } else {
        WriteLog("error", "[transfer.sh] Upload failed. Response code: " + nm.responseCode());
    }

    return 0;
}