function UploadFile(fileName, options) {
    local token = ServerParams.getParam("token");
    if (token == "") {
        WriteLog("error", "[take-me-to.space] Token not set!");
        return 0;
    }
    
    local task = options.getTask().getFileTask();
    local name = task.getDisplayName();
    local mime = GetFileMimeType(name);
    
    nm.setUrl("https://take-me-to.space/api/upload");
    
    nm.addQueryHeader("token", token);
    nm.addQueryHeader("albumid", ServerParams.getParam("albumid"));
    nm.addQueryHeader("filelength", ServerParams.getParam("filelength"));
    nm.addQueryHeader("age", ServerParams.getParam("age"));
    nm.addQueryHeader("striptags", ServerParams.getParam("striptags"));
    
    nm.addQueryParamFile("files[]", fileName, name, mime);
    
    nm.doUploadMultipartData();
    
    if (nm.responseCode() == 200) {
        local sJSON = nm.responseBody();
        local t = ParseJSON(sJSON);
        if (t != null) {
            if ("files" in t && t.files.len() > 0) {
                local file = t.files[0];
                if ("url" in file) {
                    options.setDirectUrl(file.url);
                }
                if ("deleteUrl" in file) {
                    options.setDeleteUrl("https://take-me-to.space" + file.deleteUrl);
                }
                return 1;
            } else {
                WriteLog("error", "[take-me-to.space] Unexpected response format");
            }
        }
    } else {
        WriteLog("error", "[take-me-to.space] HTTP error: " + nm.responseCode());
    }
    return 0;
}

function GetServerParamList() {
    return {
        token = "API Token",
        albumid = "Album ID (optional)",
        age = "Age (optional)",
        striptags = "Strip Tags (optional)"
    }
}