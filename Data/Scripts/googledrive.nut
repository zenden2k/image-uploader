clientSecret <- "65ie-G5nWqGMv_THtY3z2snZ";
clientId <- "162038470312-dn0kut9j7l0cd9lt32r09j0c841goei9.apps.googleusercontent.com";

function _GetAuthorizationString() {
    return ServerParams.getParam("tokenType") + " " + ServerParams.getParam("token");
}

function _ClearAuthData() {
    ServerParams.setParam("token", "");
    ServerParams.setParam("expiresIn", "");
    ServerParams.setParam("refreshToken", "");
    ServerParams.setParam("tokenType", "");
    ServerParams.setParam("tokenTime", "");
}

function _CheckResponse(except = false) {
    if (nm.responseCode() == 403) {
        if (nm.responseBody().find("Invalid token", 0)!= null) {
            WriteLog("warning", nm.responseBody());
            _ClearAuthData();
            if (except) {
                throw "unauthorized_exception";
            }
            return -2;
        } else {
            WriteLog("error", "403 Access denied" );
        }
        return 0;
    } else if (nm.responseCode() == 401)  {
        _ClearAuthData();
        if (except) {
            throw "unauthorized_exception";
        } 
        return -2;
    } else if (nm.responseCode() == 400) {
        local t = ParseJSON(nm.responseBody());
        if ("error" in t && t.error == "invalid_grant") {
            _ClearAuthData();
            return -2;
        }
    } else if (/*nm.responseCode() == 0 ||*/ (nm.responseCode() >= 400 && nm.responseCode() <= 499)) {
        WriteLog("error", "Response code " + nm.responseCode() + "\r\n" + nm.errorString() );
        return 0;
    }
    return 1;
}

function RefreshToken() {
    local token = ServerParams.getParam("token");
    local tokenType = ServerParams.getParam("tokenType");

    if (token != "" && tokenType != "") {
        local tokenTime  = 0;
        local expiresIn = 0;
        local refreshToken = "";
        try {
            tokenTime = ServerParams.getParam("tokenTime").tointeger();
        } catch ( ex ) {

        }
        try {
            expiresIn = ServerParams.getParam("expiresIn").tointeger();
        } catch ( ex ) {

        }
        refreshToken = ServerParams.getParam("refreshToken");
        if (time() > tokenTime + expiresIn && refreshToken != "") {
            // Refresh access token
            nm.setUrl("https://www.googleapis.com/oauth2/v3/token");
            nm.addQueryParam("refresh_token", refreshToken);
            nm.addQueryParam("client_id", clientId);
            nm.addQueryParam("client_secret", clientSecret);
            nm.addQueryParam("grant_type", "refresh_token");
            nm.doPost("");
            local code = _CheckResponse();
            if (code < 1) {
                WriteLog("error", "[googledrive] refreshing token failed, response code: " + nm.responseCode());
                return code;
            } else {
                local data = nm.responseBody();
                local t = ParseJSON(data);
                if ("access_token" in t) {
                    token = t.access_token;
                    if ( token != "" ) {
                        ServerParams.setParam("expiresIn", t.expires_in);
                        ServerParams.setParam("tokenType", t.token_type);
                        ServerParams.setParam("token", token);
                        ServerParams.setParam("tokenTime", time().tostring());
                        return 1;
                    }
                }
                return 0;
            }
        } else {
            return 1;
        }
        return 1;
    }
    return 0;
}

function Authenticate() {
    local login = ServerParams.getParam("Login");
    local scope = "https://www.googleapis.com/auth/drive.file";

    if (login == "") {
        WriteLog("error", "E-mail should not be empty!");
        return 0;
    }

    if (RefreshToken()){
        return 1;
    }

    local server = WebServer();
    local confirmCode = "";

    server.resource("^/$", "GET", function(d) {
        local responseBody = "";
        if ("code" in d.queryParams){
            confirmCode = d.queryParams.code;
            responseBody = "<h1>" + tr("googledrive.oauth.title", "Authorization") + "</h1><p>" + tr("googledrive.oauth.success", "Success! Now you can close this page.")+"</p>";
        } else {
            responseBody = "<h1>" + tr("googledrive.oauth.title", "Authorization") + "</h1><p>" + tr("googledrive.oauth.success", "Failed to obtain confirmation code") + "</p>";
        }

        return {
            responseBody = responseBody,
            stopDelay = 500
        };
    }, null);

    local port = server.bind(0);

    local redirectUrl = "http://127.0.0.1:" + port + "/";

    local url = "https://accounts.google.com/o/oauth2/auth?scope="+ nm.urlEncode(scope) +"&redirect_uri="+nm.urlEncode(redirectUrl)+"&response_type=code&"+ "client_id="+clientId;
    ShellOpenUrl(url);

    server.start();

    if (confirmCode == "") {
        WriteLog("error", "Cannot authenticate without confirm code");
        return 0;
    }

    nm.setUrl("https://www.googleapis.com/oauth2/v3/token");
    nm.addQueryParam("code", confirmCode);
    nm.addQueryParam("client_id", clientId);
    nm.addQueryParam("client_secret", clientSecret);
    nm.addQueryParam("redirect_uri", redirectUrl);
    nm.addQueryParam("grant_type", "authorization_code");
    nm.doPost("");
    local code = _CheckResponse();
    if (code < 1) {
        WriteLog("error", "[googledrive] Auth failed, response code: " + nm.responseCode());
        return 0;
    }
    local data = nm.responseBody();
    local t = ParseJSON(data);
    local timestamp = time();

    if ("access_token" in t) {
        ServerParams.setParam("token", t.access_token);
        ServerParams.setParam("expiresIn", t.expires_in);
        ServerParams.setParam("refreshToken", t.refresh_token);
        ServerParams.setParam("tokenType", t.token_type);
        ServerParams.setParam("tokenTime", ""+timestamp);
        return 1;
    } else {
        WriteLog("error", "Authentication failed");
    }
    return 0;
}

function IsAuthenticated() {
    if (ServerParams.getParam("token") != "") {
        return 1;
    }
    return 0;
}

function DoLogout() {
    local token = ServerParams.getParam("token");
    if (token == "") {
        return 0;
    }
    nm.setUrl("https://accounts.google.com/o/oauth2/revoke?token=" + nm.urlEncode(token));
    nm.doPost("");

    if (nm.responseCode() == 200) {
        ServerParams.setParam("token", "");
        ServerParams.setParam("refreshToken", "");
        return 1;
    } else {
        local t = ParseJSON(nm.responseBody());

        if ("error" in t && t.error == "invalid_token") {
            ServerParams.setParam("token", "");
            ServerParams.setParam("refreshToken", "");
            return 1;
        }
    }
    return 0;
}

function GetFolderList(list) {
    nm.addQueryHeader("Authorization", _GetAuthorizationString());
    nm.doGet("https://www.googleapis.com/drive/v2/files");
    local code = _CheckResponse();
    if (code < 1) {
        WriteLog("error", "[googledrive] Getting folder list failed, response code: " + nm.responseCode());
        return code;
    } else {
        local t = ParseJSON(nm.responseBody());
        if ( t != null ) {
            local count = t.items.len();
            for ( local i = 0; i < count; i++ ) {
                local item = t.items[i];
                if ( item.mimeType != "application/vnd.google-apps.folder" ) {
                    continue;
                }
                local folder = CFolderItem();
                folder.setId(item.id);
                folder.setTitle(item.title);
                //folder.setSummary(summary);
                folder.setViewUrl(item.alternateLink);
                list.AddFolderItem(folder);
            }
            return 1;
        }
    }

    return 0;
}

function CreateFolder(parentFolder, folder) {
    nm.setUrl("https://www.googleapis.com/drive/v2/files");
    nm.addQueryHeader("Authorization", _GetAuthorizationString());

    local data = {
        title = folder.getTitle(),
        mimeType = "application/vnd.google-apps.folder"
    };
    nm.addQueryHeader("Content-Type","application/json");
    nm.doPost(ToJSON(data));
    local code = _CheckResponse();
    if (code < 1) {
        WriteLog("error", "[googledrive] Creating folder failed, response code: " + nm.responseCode());
        return code;
    } else {
        local responseData = nm.responseBody();
        local item = ParseJSON(responseData);
        if ( item != null ) {
            local folder = CFolderItem();
            folder.setId(item.id);
            folder.setTitle(item.title);
            folder.setViewUrl(item.alternateLink);
            return 1;
        }
    }
    return 0;
}

function ModifyFolder(folder) {
    nm.setMethod("PUT");
    nm.setUrl("https://www.googleapis.com/drive/v2/files/" + folder.getId());
    nm.addQueryHeader("Authorization", _GetAuthorizationString());
    nm.addQueryHeader("Content-Type", "application/json");
    local postData = {
        title = folder.getTitle(),
    };
    nm.doUpload("", ToJSON(postData));
    local code = _CheckResponse();
    if (code < 1) {
        WriteLog("error", "[googledrive] Modifying folder failed, response code: " + nm.responseCode());
        return code;
    }

    return 1;
}

function UploadFile(FileName, options) {
    local fileSizeStr = GetFileSizeDouble(FileName).tostring();
    local mimeType = GetFileMimeType(FileName);
    nm.addQueryHeader("Content-Type", "application/json; charset=UTF-8");
    nm.addQueryHeader("X-Upload-Content-Type", mimeType);
    nm.addQueryHeader("X-Upload-Content-Length", fileSizeStr);
    nm.setCurlOptionInt(52, 0); //disable CURLOPT_FOLLOWLOCATION
    local postData = {
        title = ExtractFileName(FileName),
        parents = [],
    };
    local folderId = options.getFolderID();
    if (folderId != "") {
        postData.parents = [ {id = folderId, kind = "drive#parentReference"}];
    }
    local str = ToJSON(postData);

    nm.setUrl("https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable");
    nm.addQueryHeader("Authorization", _GetAuthorizationString());
    nm.setMethod("POST");
    nm.doPost(str);
    local code = _CheckResponse();
    if (code < 1) {
        WriteLog("error", "[googledrive] Starting upload failed, response code: " + nm.responseCode());
        return code;
    } else {
        local sessionUri = nm.responseHeaderByName("Location");
        if (sessionUri != "") {
            nm.setMethod("PUT");
            nm.addQueryHeader("Authorization", _GetAuthorizationString());
            nm.addQueryHeader("Content-Type", mimeType);
            //nm.addQueryHeader("Content-Length", fileSizeStr);
            nm.setUrl(sessionUri);
            nm.doUpload(FileName, "");
            code = _CheckResponse();
            if (code < 1) {
                WriteLog("error", "[googledrive] Starting upload failed, response code: " + nm.responseCode());
                return code;
            } else {
                    local responseData = nm.responseBody();
                    local item = ParseJSON(responseData);
                    nm.addQueryHeader("Authorization", _GetAuthorizationString());
                    nm.setUrl("https://www.googleapis.com/drive/v2/files/"+ item.id + "/permissions");
                    local postData = {
                        role = "reader",
                        type = "anyone",
                    };
                    nm.addQueryHeader("Content-Type", "application/json");
                    nm.setMethod("POST");
                    nm.doPost(ToJSON(postData));
                    options.setViewUrl(item.alternateLink);

                    if (item.mimeType.find("image/", 0) == 0) {
                        // There is not such URL in API response. It may break in the future.
                        options.setDirectUrl("https://drive.google.com/uc?export=view&id=" + item.id);
                    }
                    
                    if ("thumbnailLink" in item) {
                        options.setThumbUrl(item.thumbnailLink);
                    }

                    /*try {
                        options.setDirectUrl(item.webContentLink);
                    } catch ( ex ) {

                    }*/
                    return 1;
            }
        }
        return 0;
    }

    return 0;
}
