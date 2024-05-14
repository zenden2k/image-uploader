function getThumbnailWidth(){
    local result = "180";
    try{
        result = options.getParam("THUMBWIDTH");
    } catch(ex) {
    }
    return result;
}

function UploadFile(FileName, options){    
    local login = ServerParams.getParam("Login");
    local pass = ServerParams.getParam("Password");
    if (login == "" || pass == "") {
        WriteLog("error", "imageban.ru: Login or Password cannot be empty.\r\nYou must set Login and Password in server settings.");
        return 0;
    }
    
    nm.setUrl("https://imageban.ru/up");
    nm.addQueryHeader("User-Agent", "Shockwave Flash");
    nm.addQueryParam("compmenu", "0");
    nm.addQueryParam("albmenu", "0");
    nm.addQueryParam("inf", "1");
    nm.addQueryParam("cat", "0");
    nm.addQueryParam("prew", getThumbnailWidth());
    nm.addQueryParam("ttl", "0");
    nm.addQueryParam("ptext", "Увеличить");
    nm.addQueryParam("itext", "");
    nm.addQueryParam("grad", "0");
    nm.addQueryParam("rsize", "1");
    nm.addQueryParamFile("Filedata", FileName, ExtractFileName(FileName), "");
    nm.addQueryHeader("Cookie", "login="+login+"; pass="+md5(pass));
    
    nm.doUploadMultipartData();
    
    if (nm.responseCode() == 200) {
        local t = ParseJSON(nm.responseBody());
        if ("files" in t && t.files.len()) {
            local file = t.files[0];
            if ("error" in file) {
                if (file.error == "Only registered users can upload images"){
                    WriteLog("error", "imageban.ru: Invalid login or password");
                } else {
                    WriteLog("error", "imageban.ru: " + file.error);
                }
                return 0;
            }
            if (!("link" in file) || file.link == "") {
                WriteLog("error", "imageban.ru: Getting link failed");
                return 0;
            }
            options.setDirectUrl(file.link);
            
            if ("thumbs" in file) {
                options.setThumbUrl(file.thumbs);
            }
            if ("piclink" in file) {
                options.setViewUrl(file.piclink);
            }
            if ("delete" in file) {
                options.setDeleteUrl(file.rawget("delete"));
            }            
            return 1; // Success
        } else {
            WriteLog("error", "imageban.ru: Unknown error");
        }
    } else {
        WriteLog("error", "imageban.ru: Upload failed. Response code: " + nm.responseCode());
    }
    return 0;
}
