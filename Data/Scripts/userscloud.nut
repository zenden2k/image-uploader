const BASE_HOST = "https://userscloud.com/";

function _getUploadURL() {
    nm.doGet(BASE_HOST);
    if (nm.responseCode() != 200) {
        WriteLog("error", "[userscloud.com] Cannot obtain upload server address. \r\nResponse code:" + nm.responseCode());
        return "";
    }

    local doc = Document(nm.responseBody());
    local uploadURL = doc.find("#uploadfile").attr("action");
    if (uploadURL != "") {
        return uploadURL;
    } 
    WriteLog("error", "[userscloud.com] Cannot obtain upload server address from the main page.");
    return "";
}

function UploadFile(fileName, options) {
    local uploadURL = _getUploadURL();
    if (uploadURL == "") {
        return 0;
    }
    local name = ExtractFileName(fileName);
    local mime = GetFileMimeType(name);
    nm.setUrl(uploadURL);
    nm.addQueryParam("sess_id", "");
    nm.addQueryParam("utype", "anon");
    nm.addQueryParam("link_rcpt", "");
    nm.addQueryParam("link_pass", "");
    nm.addQueryParam("keepalive", "1");
    nm.addQueryParamFile("file_0", fileName, name, mime);
    nm.doUploadMultipartData();

    if (nm.responseCode() == 200) {
        local sJSON = nm.responseBody();
        local t = ParseJSON(sJSON);
        if (t != null) {
            if (t[0].file_status = "Ok") {
                nm.doGet(BASE_HOST + "?op=upload_result&st=OK&fn=" + nm.urlEncode(t[0].file_code));

                if (nm.responseCode() == 200) {
                    local doc = Document(nm.responseBody());
                    local viewURL = strip(doc.find("#tab1 textarea").text());
                    local directURL = strip(doc.find("#tab5 textarea").text());
                    local thumbURL = strip(doc.find("#tab6 textarea").text());
                    local deleteURL = strip(doc.find("#tab4 textarea").text());
                    options.setDirectUrl(directURL);
                    options.setViewUrl(viewURL);
                    options.setThumbUrl(thumbURL);
                    options.setDeleteUrl(deleteURL);

                    if (directURL != "" || viewURL != "") {
                        return 1;
                    } else {
                        WriteLog("error", "[userscloud.com] Cannot obtain file links from server's response.");
                    }
                }
                
                return 1;
            } else {
                WriteLog("error", "[userscloud.com] Status: " + t.file_status);
            }
        } 
    } else {
        WriteLog("error", "[userscloud.com] Failed to upload. \r\nResponse code: " + nm.responseCode());
    }

    return 0;
}