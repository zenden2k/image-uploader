callbackUrl <- "http://zenden2k.com/callback";
clientId <- "nxmmubtkn2mkfk7cjj3q";
userAgent <- "Zenden2k Image Uploader";

function BeginLogin() {
    try {
        return Sync.beginAuth();
    }
    catch ( ex ) {
    }
    return false;
}

function EndLogin() {
    try {
        return Sync.endAuth();
    } catch ( ex ) { 
    }
    return false;
}

function _DoLogin() { 
    local clientSecret = "4dj3du5476qyd62s4xpgd46ap80k9h6n29ufpj2b";
    local login = ServerParams.getParam("Login"); 
    if (login == "") {
        return 1;
    }    
    local token = ServerParams.getParam("token");
    if (token != "") {
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
        
        if ( time() > tokenTime + expiresIn && refreshToken != "") {
            nm.setUrl("https://auth.teknik.io/connect/token");
            nm.setUserAgent(userAgent);
            nm.addQueryParam("client_id", clientId);
            nm.addQueryParam("client_secret", clientSecret);
            nm.addQueryParam("grant_type", "refresh_token");
            nm.addQueryParam("refresh_token", refreshToken);
            nm.doPost("");
            
            if (nm.responseCode() == 200) {
                local t = ParseJSON(nm.responseBody());
                if ("access_token" in t) {
                    ServerParams.setParam("token", t.access_token);
                    ServerParams.setParam("tokenType", t.token_type);
                    ServerParams.setParam("expiresIn", t.expires_in);
                    ServerParams.setParam("tokenTime", time().tostring());
                    ServerParams.setParam("refreshToken", t.refresh_token);
                    return 1;
                }
            }
            return 0;
        }
        return 1;
    } 
    local scope = "openid offline_access teknik-api.write";
    ShellOpenUrl("https://auth.teknik.io/connect/authorize/callback?response_type=code&redirect_uri="+ nm.urlEncode(callbackUrl) + "&scope=" + nm.urlEncode(scope) + "&client_id=" + clientId);
	
	local confirmCode = InputDialog("You need to need to sign in to your tenkinik.io account in web browser\r\nwhich just has been opened and then copy confirmation code into the text field below. \r\nPlease enter the confirmation code:", "");
	
	if (confirmCode == "") {
		WriteLog("error", "Cannot contniue without confirmation code!");
		return 0;
	}

    nm.setUrl("https://auth.teknik.io/connect/token");
    nm.setUserAgent(userAgent);
    nm.addQueryParam("client_id", clientId);
    nm.addQueryParam("client_secret", clientSecret);
    nm.addQueryParam("grant_type", "authorization_code");
    nm.addQueryParam("redirect_uri", callbackUrl);
    nm.addQueryParam("code", confirmCode);
    nm.doPost("");
    
    if (nm.responseCode() == 200) {
        local t = ParseJSON(nm.responseBody());
        if ("access_token" in t) {
            ServerParams.setParam("token", t.access_token);
            ServerParams.setParam("tokenType", t.token_type);
            ServerParams.setParam("expiresIn", t.expires_in);
            ServerParams.setParam("tokenTime", time().tostring());
            ServerParams.setParam("refreshToken", t.refresh_token);
            return 1;
        }
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

function GetAuthorizationString() {
	local token = ServerParams.getParam("token");
	local tokenType = ServerParams.getParam("tokenType");
	return tokenType + " " + token;
}

function UploadFile(FileName, options) { 
    if (!DoLogin()) {
        return 0;
    }
    
    nm.setUrl("https://api.teknik.io/v1/Upload");
    nm.setUserAgent(userAgent);
    nm.addQueryParamFile("file", FileName, ExtractFileName(FileName), "");
    nm.addQueryParam("genDeletionKey", "true");
    nm.addQueryHeader("Authorization", GetAuthorizationString());
    nm.doUploadMultipartData();
    
    if ( nm.responseCode() == 200) {
        local t = ParseJSON(nm.responseBody());
        if ("result" in t ) {
            if ("deletionKey" in t.result) {
                options.setDeleteUrl(t.result.url + "/" + t.result.deletionKey);
            }
            options.setDirectUrl(t.result.url);
            return 1;
        }
    }

    return 0;
}


function  ShortenUrl(url, options) {	
    if (!DoLogin()) {
        return 0;
    }
    nm.setUrl("https://api.teknik.io/v1/Shorten");
    nm.setUserAgent(userAgent);
    nm.addQueryParam("url", url);
    nm.addQueryHeader("Authorization", GetAuthorizationString());
    nm.doPost("");
    
    if (nm.responseCode() == 200) {
        local t = ParseJSON(nm.responseBody());
        if ("result" in t) {
            options.setDirectUrl(t.result.shortUrl);
            return 1;
        } else if ("error" in t) {
            WriteLog("error", "teknik.io: " + t.error.message);
        }
    }
	return 0;
}

/*function GetServerParamList()
{	
	return {
		token = "token",
		tokenType = "tokenType",
		expiresIn = "expiresIn",
		refreshToken = "refreshToken",
		tokenTime = "tokenTime"
	};
}*/
