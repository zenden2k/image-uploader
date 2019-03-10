apiHostName <- "https://api.jheberg.net";
accessToken <- "";
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

    local username = ServerParams.getParam("Login");
    if (username == "" ) {
        return 1;
    }
    local pass =  ServerParams.getParam("Password");
        
    local requestData = {
        username = username,
        password = pass,
    };
    nm.addQueryHeader("Content-Type", "application/json");
    nm.setUrl(apiHostName + "/me");
    nm.doPost(ToJSON(requestData));
    if (nm.responseCode() == 200 ) {
        local data =  nm.responseBody();
        local jsonData = ParseJSON(data);
        if ( "token" in jsonData) {
            accessToken = jsonData.token;
            ServerParams.setParam("accessToken",accessToken);
            return 1;
        } else {
            WriteLog("error", "jheberg.net: Login failed using login ' " + username + ". HTTP responseCode = " + nm.responseCode());
        }
    } else {
        WriteLog("error", "jheberg.net: Unable to authentificate using login '" + username + "'. HTTP responseCode = " + nm.responseCode());
    }
        
	return 0;
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
    if (!DoLogin()) {
        return 0;
    }
    
    local uploadServer = Sync.getValue("uploadServer")
    if (uploadServer == "") {
        nm.doGet(apiHostName + "/backend?type=UPLOAD");
        if ( nm.responseCode() != 200) {
            WriteLog("error", "jheberg.net error: Failed to find server for uploading.");  
            return 0;         
        }
        local d = ParseJSON(nm.responseBody());
        local selectedServer = d[random() % d.len()];
        uploadServer = selectedServer.url;
        Sync.setValue("uploadServer", uploadServer);
    }
    
    if (accessToken != ""){
        nm.addQueryHeader("X-Auth-Token", accessToken);
    }
    nm.setUrl(uploadServer + "file");
    nm.addQueryParam("uniqueId", time().tostring());
    local hosters = [109, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107];
    foreach (hoster in hosters) {
        nm.addQueryParam("hosters", hoster.tostring());
    }
    nm.addQueryParamFile("file", FileName, ExtractFileName(FileName),"");
    nm.doUploadMultipartData();
    if (nm.responseCode() == 200) {
        local serverData = nm.responseBody();
        local obj = ParseJSON(serverData);
        
        if ( "id" in obj) {
            options.setViewUrl("https://download.jheberg.net/" + obj.id);
            return 1;
        } else {
            WriteLog("error", "jheberg.net error: Upload failed.");    
            return 0;
        }
    } else {
        WriteLog("error", "jheberg.net error: Invalid response code.");
    }
    return 0;
}