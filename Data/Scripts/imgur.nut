/*
    Image Uploader scriptable add-on
    Authors: Alexamder Mikhnevich aka @arhangelsoft at github.com
             Sergey Svistunov @zenden2k
*/

const CLIENT_ID = "b38ba6b0919a898";

function GetAuthorizationString() {
    local token = ServerParams.getParam("token");
    local tokenType = ServerParams.getParam("tokenType");
    if (token == "") {
        return "Client-ID " + CLIENT_ID;
    }
    return "Bearer" + " " + token;
}

function IsAuthenticated() {
    return ServerParams.getParam("token") != "" ? 1 : 0;
}

function checkResponse(except = false) {
    if ( nm.responseCode() == 403 ) {
        if ( nm.responseBody().find("Invalid token",0)!= null) {
            WriteLog("warning", nm.responseBody());
            ServerParams.setParam("token", "");
            ServerParams.setParam("expiresIn", "");
            ServerParams.setParam("refreshToken", "");
            ServerParams.setParam("tokenType", "");
            ServerParams.setParam("prevLogin", "");
            ServerParams.setParam("tokenTime", "");
            if (except) {
                throw "unauthorized_exception";
            }
            return -2;
        } else {
            WriteLog("error", "403 Access denied" );
            return 0;
        }
    } else if ( (nm.responseCode() >= 400 && nm.responseCode() <= 499)) {
        WriteLog("error", "Response code " + nm.responseCode() + "\r\n" + nm.errorString() );
        return 0;
    }
    return 1;
}

function RefreshToken() {
    local login = ServerParams.getParam("Login");
    if (login == "") {
        return 1;
    }
    local token = ServerParams.getParam("token");
    local tokenType = ServerParams.getParam("tokenType");
    if (token != "" && tokenType != "" ) {
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
            nm.setUrl("https://api.imgur.com/oauth2/token");
            nm.addQueryParam("refresh_token", refreshToken); 
            nm.addQueryParam("client_id", CLIENT_ID); 
            nm.addQueryParam("client_secret", "d91cd90f01cadaa1796d8d9b9231c218c11ed628"); 
            nm.addQueryParam("grant_type", "refresh_token"); 
            nm.doPost("");
            if (checkResponse()) {
                local data =  nm.responseBody();
                local t = ParseJSON(data);
                if ("access_token" in t) {
                    token = t.access_token;
                    ServerParams.setParam("expiresIn", t.expires_in);
                    tokenType = t.token_type;
                    ServerParams.setParam("tokenType", tokenType);
                    ServerParams.setParam("token", token);
                    ServerParams.setParam("tokenTime", time().tostring());
                    ServerParams.setParam("refreshToken", t.refresh_token);
                    if ( token != "" ) {
                        return 1;
                    } else {
                        token = "";
                        tokenType = "";
                        return 0;
                    }
                } else {
                    WriteLog("error", "imgur: Unable to refresh access token.");
                    return 0;
                }
            } else {
                WriteLog("error", "imgur: Unable to refresh access token.");
            }
        } else {
            return 1;
        }
    }
    return 0;
}

function Authenticate()  {
    local login = ServerParams.getParam("Login");
    if (login == "" || ServerParams.getParam("token") != "") {
        return 1;
    } 
    local login = ServerParams.getParam("Login");
    local url = "https://api.imgur.com/oauth2/authorize?client_id=" + nm.urlEncode(CLIENT_ID) +"&response_type=token&state=token";
    ShellOpenUrl(url);
    
    local confirmCode = InputDialog(tr("imgur.confirmation.text", "You need to need to sign in to your Imgur account\r\n in web browser which just have opened and then\r\n copy confirmation code into the text field below.\r\nPlease enter confirmation code:"),"");
    
    if ( confirmCode == "" ) {
        WriteLog("error", "Cannot authenticate without confirm code");
        return 0;
    }
    
    local t = ParseJSON(confirmCode);
    
    if (t != null && "access_token" in t) {
        ServerParams.setParam("token", t.access_token);
        ServerParams.setParam("expiresIn", t.expires_in);
        ServerParams.setParam("refreshToken", t.refresh_token);
        ServerParams.setParam("tokenType", t.token_type);
        ServerParams.setParam("tokenTime", t.timestamp);
        ServerParams.setParam("accountId", t.account_id);
        return 1;
    } else {
        WriteLog("Invalid confirmation code");
    }
 
    return 0;        
} 

function UploadFile(FileName, options) {	
    local login = ServerParams.getParam("Login");
    nm.setUrl("https://api.imgur.com/3/image");
    nm.addQueryHeader("Authorization", GetAuthorizationString());
    nm.addQueryParamFile("image", FileName, ExtractFileName(FileName),"");
    nm.doUploadMultipartData();
    local code = checkResponse();
    if (code < 1) {
        return code;
    } else if (nm.responseCode() == 200) {
        local retdoc = nm.responseBody();
        local json = ParseJSON(retdoc);
        if (json != null) {
            if (json.success == true) {
                local directUrl = json.data.link;
                local id = json.data.id;
                options.setDirectUrl(directUrl);
                options.setViewUrl("https://imgur.com/" + id);
                return 1;
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    } else {
        return 0;
    }
}

/*function GetServerParamList(){
    return {
        token = "token",
        tokenType = "tokenType",
        expiresIn = "expiresIn",
        refreshToken = "refreshToken",
        tokenTime = "tokenTime"
    }
}*/