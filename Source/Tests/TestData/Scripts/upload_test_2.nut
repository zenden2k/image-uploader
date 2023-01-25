// This test script is used in ScriptUploadEngineTest
function Authenticate() {
    nm.setUrl("https://example.com/login");
    nm.addQueryParam("login", ServerParams.getParam("Login"));
    nm.addQueryParam("password", ServerParams.getParam("Password"));
    if (nm.doPost("")){
        Sync.setValue("token", "testtoken");
        return 1;
    }
    return 0;
}

function IsAuthenticated() {
    if (ServerParams.getParam("token") != "") {
        return 1;
    }
    return 0;
}

function UploadFile(FileName, options) {
    local name = ExtractFileName(FileName);
    local mime = GetFileMimeType(name);
    nm.setUrl("https://example.com/upload");
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
                    WriteLog("error", "Upload Test 1: " + t.error.message);
                }
            }
        } 
    }
    
    return 0;
}