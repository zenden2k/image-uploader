// Chevereto uploader
function UploadFile(FileName, options) {
    local name = ExtractFileName(FileName);
    local mime = GetFileMimeType(name);
    local login = ServerParams.getParam("Login");
    local pass =  ServerParams.getParam("Password");

    nm.setUrl("https://lostpic.net/api/1/upload");
    nm.addQueryParam("key", "8fda0f020c89a1fe53d117deef24ab03");
    if ( login != "" && pass != "") {
        nm.addQueryParam("login", login);
        nm.addQueryParam("password", pass);
    }
    nm.addQueryParam("format", "json");
    nm.addQueryParamFile("source", FileName, name, mime);
	nm.doUploadMultipartData();
    
    if (nm.responseCode() > 0) {
        local sJSON = nm.responseBody();
        local t = ParseJSON(sJSON);
        if (t != null) {
            if ("status_code" in t && t.status_code == 200){
                options.setViewUrl(t.image.url_viewer );
                options.setThumbUrl(t.image.thumb.url);
                options.setDirectUrl(t.image.url);
                return 1;
            } else {
                if("error" in t && t.error!=null) {
                    WriteLog("error", "lostpic.net: " + t.error.message);
                }
            }
        }
    }        
    
    return 0;
}