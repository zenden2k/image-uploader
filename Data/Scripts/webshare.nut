function _StrReplace(str, pattern, replace_with) {
    local resultStr = str;
    local res;
    local start = 0;

    while( (res = resultStr.find(pattern,start)) != null ) {

        resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
        start = res + replace_with.len();
    }
    return resultStr;
}

// Get salt for authentication
function _GetSalt() {
    local login = ServerParams.getParam("Login");
    if (login == "") {
        WriteLog("error", "Webshare.cz: Login not specified");
        return "";
    }
    
    nm.setUrl("https://webshare.cz/api/salt/");
    nm.setUserAgent("Webshare klient/1.0 (Windows NT 10.0; Win64; x64)");
    nm.addQueryParam("username_or_email", login);
    
    if (!nm.doPost("")) {
        WriteLog("error", "Webshare.cz: Failed to get salt");
        return "";
    }
    
    local xml = SimpleXml();
    xml.LoadFromString(nm.responseBody());
    local root = xml.GetRoot("response", false);
    
    if (root.IsNull()) {
        WriteLog("error", "Webshare.cz: Invalid response when getting salt");
        return "";
    }
    
    local statusNode = root.GetChild("status", false);
    if (statusNode.IsNull() || statusNode.Text() != "OK") {
        WriteLog("error", "Webshare.cz: Error status when getting salt");
        return "";
    }
    
    local saltNode = root.GetChild("salt", false);
    if (saltNode.IsNull()) {
        WriteLog("error", "Webshare.cz: Salt not found in response");
        return "";
    }
    
    return saltNode.Text();
}

// Get upload URL
function _GetUploadUrl(token) {
    local uploadUrl = Sync.getValue("uploadUrl");

    if (uploadUrl != "") {
        return uploadUrl;
    }

    nm.setUrl("https://webshare.cz/api/upload_url/");
    nm.setUserAgent("Webshare klient/1.0 (Windows NT 10.0; Win64; x64)");
    nm.addQueryParam("wst", token);
    
    if (!nm.doPost("")) {
        WriteLog("error", "Webshare.cz: Failed to get upload URL");
        return null;
    }
    
    local xml = SimpleXml();
    xml.LoadFromString(nm.responseBody());
    local root = xml.GetRoot("response", false);
    
    if (root.IsNull()) {
        WriteLog("error", "Webshare.cz: Invalid response when getting upload URL");
        return null;
    }
    
    local statusNode = root.GetChild("status", false);
    if (statusNode.IsNull() || statusNode.Text() != "OK") {
        WriteLog("error", "Webshare.cz: Error status when getting upload URL");
        return null;
    }
    
    local urlNode = root.GetChild("url", false);
    if (urlNode.IsNull()) {
        WriteLog("error", "Webshare.cz: Upload URL not found in response");
        return null;
    }
    
    uploadUrl = urlNode.Text();
    Sync.setValue("uploadUrl", uploadUrl);

    return uploadUrl;
}

// Authentication function - called automatically by Image Uploader
function Authenticate() {
    local login = ServerParams.getParam("Login");
    local password = ServerParams.getParam("Password");
    
    if (login == "" || password == "") {
        WriteLog("error", "Webshare.cz: Login or password not specified");
        return ResultCode.Failure;
    }
    
    // Get salt first
    local salt = _GetSalt();
    if (salt == "") {
        return ResultCode.Failure;
    }
    
    // Generate password hash
    local passwordHash = Sha1(Md5Crypt(password, salt));
    
    nm.setUrl("https://webshare.cz/api/login/");
    nm.setUserAgent("Webshare klient/1.0 (Windows NT 10.0; Win64; x64)");
    nm.addQueryParam("username_or_email", login);
    nm.addQueryParam("password", passwordHash);
    nm.addQueryParam("keep_logged_in", "1");
    
    if (!nm.doPost("")) {
        WriteLog("error", "Webshare.cz: Authentication failed");
        return ResultCode.Failure;
    }
    
    local xml = SimpleXml();
    xml.LoadFromString(nm.responseBody());
    local root = xml.GetRoot("response", false);
    
    if (root.IsNull()) {
        WriteLog("error", "Webshare.cz: Invalid response during authentication");
        return ResultCode.Failure;
    }
    
    local statusNode = root.GetChild("status", false);
    if (statusNode.IsNull() || statusNode.Text() != "OK") {
        WriteLog("error", "Webshare.cz: Authentication failed - invalid status");
        return ResultCode.Failure;
    }
    
    local tokenNode = root.GetChild("token", false);
    if (tokenNode.IsNull()) {
        WriteLog("error", "Webshare.cz: Token not found in response");
        return ResultCode.Failure;
    }
    
    local token = tokenNode.Text();

    Sync.setValue("token", token);
    return ResultCode.Success;
}

// Upload file function
function UploadFile(fileName, options) {
    local token = Sync.getValue("token");
    // Get upload URL if not already obtained
    local uploadUrl = _GetUploadUrl(token);

    if (uploadUrl == "") {
        WriteLog("error", "Webshare.cz: Failed to obtain upload URL");
        return ResultCode.Failure;
    }

    local task = options.getTask().getFileTask();
    local name = task.getDisplayName();

    nm.setUrl(uploadUrl);
    nm.setUserAgent("Webshare klient/1.0 (Windows NT 10.0; Win64; x64)");
    
    // Get file information
    local fileSize = GetFileSize(fileName);
    
    // Set form data parameters
    nm.addQueryHeader("Accept", "text/xml; charset=UTF-8");
    nm.addQueryParam("name", name);
    nm.addQueryParam("offset", "0");
    nm.addQueryParam("ident", RandomString(10));
    nm.addQueryParam("total", fileSize.tostring());
    nm.addQueryParam("wst", token);
    nm.addQueryParam("folder", "%2F"); // Root folder
    nm.addQueryParam("private", "0");
    nm.addQueryParam("adult", "0");
    nm.addQueryParamFile("file", fileName, name, GetFileMimeType(fileName));
    
    if (!nm.doUploadMultipartData()) {
        WriteLog("error", "Webshare.cz: File upload failed");
        return ResultCode.Failure;
    }
    
    // Parse response
    local responseBody = nm.responseBody();
    
    if (nm.responseCode() != 200) {
        WriteLog("error", "Webshare.cz: Failed to upload, HTTP response code:" + nm.responseCode());
        return ResultCode.Failure;
    }

    // Check if response contains XML
    if (responseBody.find("<?xml") != null) {
        local xml = SimpleXml();
        xml.LoadFromString(responseBody);
        local root = xml.GetRoot("response", false);
        
        if (!root.IsNull()) {
            local statusNode = root.GetChild("status", false);
            if (!statusNode.IsNull() && statusNode.Text() == "OK") {
                // Get ident from response to construct file URL
                local identNode = root.GetChild("ident", false);
                if (!identNode.IsNull()) {
                    local fileIdent = identNode.Text();
                    
                    // Generate file name for URL (replace dots with dashes)
                    local fileNameForUrl = _StrReplace(name, ".", "-");
                    
                    // Construct webshare.cz file URL
                    local fileUrl = "https://webshare.cz/#/file/" + fileIdent + "/" + nm.urlEncode(fileNameForUrl);
                    
                    // Set the view URL for Image Uploader
                    options.setViewUrl(fileUrl);
                    
                    return ResultCode.Success;
                } else {
                    WriteLog("error", "Webshare.cz: ident not found in response");
                    return ResultCode.Failure;
                }
            } else {
                local errorNode = root.GetChild("message", false);
                if (!errorNode.IsNull()) {
                    WriteLog("error", "Webshare.cz error: " + errorNode.Text());
                }
                return ResultCode.Failure;
            }
        }
    }
    
    WriteLog("error", "Webshare.cz: Unexpected response format");
    return ResultCode.Failure;
}