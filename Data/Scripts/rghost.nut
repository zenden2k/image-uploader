function UploadFile(FileName, options) {
    local apiKey = ServerParams.getParam("Password"); 
    local file_name = ExtractFileName(FileName);
    local file_token = "";
    local file_uploadServer = "http://muonium.rgho.st/";
    local isPremium = false;
    nm.setUserAgent("rgup 1.3");
    //nm.doGet("http://rgho.st/");
    if (apiKey != ""){
        nm.addQueryHeader("X-API-Key", apiKey);
    }
    nm.doGet("http://rghost.net/multiple/upload_host")
    if (nm.responseCode() == 200) {
        local serverData = nm.responseBody();
        local obj = ParseJSON(serverData);
        if ( "upload_host" in obj) {
            file_uploadServer = obj.upload_host;
            file_token = obj.authenticity_token;
            isPremium = obj.premium=="1" || obj.premium==true;
        } else {
            WriteLog("error", "rgho.st error: Cannot obtain upload server.");    
            return 0;
        }
        
        nm.setUrl(file_uploadServer + (isPremium? "/premium/files" : "/files"));
        if (apiKey != ""){
            nm.addQueryHeader("X-API-Key", apiKey);
        }
        nm.addQueryParamFile("file", FileName, file_name, "");
        nm.addQueryParam("authenticity_token", file_token);
        nm.setCurlOptionInt(52, 0); //disable CURLOPT_FOLLOWLOCATION 
        nm.doUploadMultipartData();
        if(nm.responseCode() == 302 ){
            local destUrl = nm.responseHeaderByName("Location");
            if (destUrl != "") {
                options.setViewUrl(destUrl);
                return 1;
            }
        } else {
            WriteLog("error", "rgho.st error: Invalid response code.");
        }
    } else {
        WriteLog("error", "rgho.st error: Invalid response code.");
    }
    return 0;
}