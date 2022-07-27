auth_token <- "";

function _ObtainToken() {
    if (auth_token == "") {
        nm.doGet("https://imgbb.com/");
        if (nm.responseCode() != 200) {
            return "";
        }
        local reg = CRegExp("auth_token=\"(.+?)\"", "");
        if ( reg.match(nm.responseBody()) ) {
            auth_token = reg.getMatch(1);
        }
    }
    return auth_token;
}

function _UploadToAccount(FileName, options) {
    nm.enableResponseCodeChecking(true);
    local apiKey = ServerParams.getParam("Login");
    nm.addQueryParam("key", apiKey);
    nm.addQueryParamFile("image", FileName, ExtractFileName(FileName), "");
    nm.setUrl("https://api.imgbb.com/1/upload");
    nm.doUploadMultipartData();
    local t = ParseJSON(nm.responseBody());
    if (nm.responseCode() == 200) { 
        if (t.success) {
            options.setViewUrl(t.data.url_viewer);
            options.setDirectUrl(t.data.url);
            options.setThumbUrl(t.data.thumb.url);
            //options.setDeleteUrl(t.data.delete_url);
            return 1;
        }
    } else if ("error" in t) {
        WriteLog("error", "[imgbb.com] got error from server: \nResponse code:" + nm.responseCode() + "\n" + t.error.message);
    }
    return 0;
}

function UploadFile(FileName, options) {
    nm.enableResponseCodeChecking(false);
    local apiKey = ServerParams.getParam("Login");

    if (apiKey != "") {
        return _UploadToAccount(FileName, options);
    }

    local name = ExtractFileName(FileName);
    local mime = GetFileMimeType(name);
    local token = _ObtainToken();
    if (token == "") {
        WriteLog("error", "[imgbb.com] Unable to obtain auth token");
        
        return 0;
    }
    nm.setUrl("https://imgbb.com/json");
    nm.addQueryParam("type", "file");
    nm.addQueryParam("action", "upload");
    nm.addQueryParam("privacy", "public");
    nm.addQueryParam("timestamp", time() + "000");
    nm.addQueryParam("auth_token", token);
    nm.addQueryParam("category_id", "");
    nm.addQueryParam("nswd", "");
    nm.addQueryParamFile("source", FileName, name, mime);
    nm.doUploadMultipartData();
    if (nm.responseCode() == 200) {
        local sJSON = nm.responseBody();
        local t = ParseJSON(sJSON);
        if (t != null) {
            options.setViewUrl(t.image.url_viewer);
            options.setDirectUrl(t.image.url);
            options.setThumbUrl(t.image.thumb.url);
            return 1;
        } else
            return 0;

    } else {
        local t = ParseJSON(nm.responseBody());
        if (t != null) {
            WriteLog("error", "[imgbb.com] got error from server: \nResponse code:" + nm.responseCode() + "\n" + t.error.message);
        }
        return 0;
    }
}