apiHostName <- "http://api.ge.tt"; // Ge.tt documentation says that we should use HTTPS for api requests, but api.ge.tt certificate has expired ....
apiKey <- "tpa2q4q14y0ddiyghink8uayvicv989cxdkkkwjeb4d2f6tuik9";
accessToken <- "";
refreshToken <- "";
expires <- 0;

function BeginLogin() {
	try {
		return Sync.beginAuth();
	}
	catch ( ex ) {
	}
	return true;
}

function EndLogin() {
	try {
		return Sync.endAuth();
	} catch ( ex ) {
		
	}
	return true;
}

function _DoLogin() {
    accessToken = ServerParams.getParam("accessToken");
    refreshToken = ServerParams.getParam("refreshToken");
    
     try {
         expires = ServerParams.getParam("expires").tointeger();
     } catch ( ex ) {
         expires = 0;
     }
    
    if (accessToken == "" || (expires <= time() || expires == 0)) {
        local email = ServerParams.getParam("Login");
        local pass =  ServerParams.getParam("Password");
        
        local requestData = {
            apikey = apiKey,
            email = email,
            password = pass,
        };
        nm.addQueryHeader("Content-Type", "application/json");
        nm.setUrl(apiHostName + "/1/users/login");
        nm.doPost(ToJSON(requestData));
        if (nm.responseCode() == 200 ) {
            local data =  nm.responseBody();
            local jsonData = ParseJSON(data);
            if ( "accesstoken" in jsonData) {
                accessToken = jsonData.accesstoken;
                refreshToken = jsonData.refreshtoken;
                expires = time() + jsonData.expires;
                ServerParams.setParam("accessToken",accessToken);
                ServerParams.setParam("refreshToken",refreshToken);
                ServerParams.setParam("expires",expires);
                return 1;
            } else {
                WriteLog("error", "ge.tt: Login failed using email ' " + email + ". HTTP responseCode = " + nm.responseCode());
            }
        } else {
            WriteLog("error", "ge.tt: Unable to authentificate using email '" + email + "'. HTTP responseCode = " + nm.responseCode());
        }
        return 0;
    }
	
	return 1;
}

function DoLogin() {
	if (!BeginLogin() ) {
		return false;
	}
	local res = _DoLogin();
	
	EndLogin();
	return res;
}

function UploadFile(FileName, options) {
    nm.setUserAgent("Zenden2k Image Uploader");
    if (!DoLogin()) {
        return 0;
    }
    nm.setUrl(apiHostName + "/1/shares/create?accesstoken=" + nm.urlEncode(accessToken));
    nm.doPost("");
    if (nm.responseCode() == 201) {
        local serverData = nm.responseBody();
        local obj = ParseJSON(serverData);
        
        if ( "sharename" in obj) {
            local shareName = obj.sharename;
            nm.setUrl(apiHostName + "/1/files/" + shareName +"/create?accesstoken=" + nm.urlEncode(accessToken));
            local data = {
                filename = ExtractFileName(FileName)
            };
            nm.addQueryHeader("Content-Type", "application/json");
            nm.doPost(ToJSON(data));
            if (nm.responseCode() == 201) {
                local obj2 = ParseJSON(nm.responseBody());
                if ("upload" in obj2) {
                    local puturl = obj2.upload.puturl;
                    nm.setUrl(puturl);
                    nm.setMethod("PUT");
                    // ge.tt documentation says that we should set Content-Length header value. 
                    // But Content-Encoding is "chunked", so...
                    nm.addQueryHeader("Transfer-Encoding", "");
                    nm.addQueryHeader("Content-Length", GetFileSize(FileName));
                    nm.doUpload(FileName, "");
                    if (nm.responseCode() == 200) {
                        options.setViewUrl("http://ge.tt/" + shareName);
                        return 1;
                    } else {
                        WriteLog("error", "ge.tt error: Failed to upload file. Response code: " + nm.responseCode()); 
                    }
                }
                
            } else {
                WriteLog("error", "ge.tt error: Cannot obtain upload server.");    
                return 0;
            }
        } else {
            WriteLog("error", "ge.tt error: Cannot create share.");    
            return 0;
        }
    } else {
        WriteLog("error", "get.tt error: Invalid response code.");
    }
    return 0;
}