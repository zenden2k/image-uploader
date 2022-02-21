appKey <- "973quph3jxdgqoe";
appSecret <- "wloizpn331cc8zd";
accessType <- "app_folder";

redirectUri <- "https://oauth.vk.com/blank.html";
redirectUrlEscaped <- "https:\\/\\/oauth\\.vk\\.com\\/blank\\.html";

authStep1Url <- "https://api.dropbox.com/1/oauth/request_token";
authStep2Url <- "https://api.dropbox.com/1/oauth/access_token";

authCode <- "";

regMatchOffset <- 0;

try {
	local ver = GetAppVersion();
	if ( ver.Build > 4422 ) {
		regMatchOffset = 1;
	}
} catch ( ex ) {
}

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

function tr(key, text) {
	try {
		return Translate(key, text);
	}
	catch(ex) {
		return text;
	}
}
	
function signRequest(url, token) {
	nm.addQueryHeader("Authorization", "Bearer " + token);
	
	return url;
}

function OnUrlChangedCallback(data) {
	local reg = CRegExp("^" + redirectUrlEscaped, "");
	if ( reg.match(data.url) ) {
		local br = data.browser;
		local regError = CRegExp("error=([^&]+)", "");
		if ( regError.match(data.url) ) {
			WriteLog("warning", regError.getMatch(regMatchOffset+0));
		} else {
			local codeRegexp = CRegExp("code=([^&]+)", "");
			if ( codeRegexp.match(data.url) ) {
				authCode = codeRegexp.getMatch(regMatchOffset+0);
			}
		}
		br.close();
	}
}
function sendOauthRequest(url, token) {
	nm.setUrl(url);
	signRequest(url, token);
	nm.doPost("" );
	return 0;
}

function RefreshToken() {
	local expiresIn = ServerParams.getParam("expiresIn").tointeger();
	local refreshToken = ServerParams.getParam("refreshToken"); 
	if (time() > expiresIn) {
		nm.setUrl("https://api.dropboxapi.com/oauth2/token");
		nm.addQueryParam("grant_type", "refresh_token");
		nm.addQueryParam("client_id", appKey);
		nm.addQueryParam("client_secret", appSecret);
		nm.addQueryParam("refresh_token", refreshToken);
		nm.doPost("");

		if (nm.responseCode() == 200) {
			local t = ParseJSON(nm.responseBody());
			ServerParams.setParam("token", t.access_token);
			ServerParams.setParam("expiresIn", t.expires_in + time());
			return 1;
		} else {
			WriteLog("error", "[dropbox.nut] Unable to refresh  token, response code: " + nm.responseCode());
			return 0;
		}
	}
	return 1;
}

function ObtainAccessToken()  {
	if (authCode != ""){
		local url = "https://api.dropboxapi.com/oauth2/token";
		nm.setUrl(url);
		nm.addQueryParam("code", authCode);
		nm.addQueryParam("grant_type", "authorization_code");
		nm.addQueryParam("redirect_uri", redirectUri);
		nm.addQueryParam("client_id", appKey);
		nm.addQueryParam("client_secret", appSecret);
		nm.doPost("");

		if (nm.responseCode() == 200) {
			local t = ParseJSON(nm.responseBody());
			ServerParams.setParam("token", t.access_token);
			ServerParams.setParam("refreshToken", t.refresh_token);
			ServerParams.setParam("expiresIn", t.expires_in + time());
			ServerParams.setParam("accountId", t.account_id);	
			ServerParams.setParam("tokenTime", time().tostring());	
			ServerParams.setParam("uid", t.uid);
			authCode = "";	
			return 1;
		} else {
			WriteLog("error", "[dropbox.nut] Unable to obtain bearer token, response code: " + nm.responseCode());
		}
	} else {
		RefreshToken();
	}
}

function _DoLogin() {
	local token = ServerParams.getParam("token");
	
	if ( token != ""){
		return 1;
	}
    
    local browser = CWebBrowser();
	browser.setTitle(tr("dropbox.browser.title", "Dropbox authorization"));
	browser.setOnUrlChangedCallback(OnUrlChangedCallback, null);
	
	local url = "https://www.dropbox.com/oauth2/authorize?" + 
			"client_id=" + appKey  + 
			"&response_type=code" +
			"&token_access_type=offline" + 
			"&redirect_uri=" + nm.urlEncode(redirectUri);

	browser.navigateToUrl(url);
	browser.showModal();
	
	return ObtainAccessToken();
}

function DoLogin() {
	if (!BeginLogin() ) {
		return false;
	}
	local res = _DoLogin();
	
	EndLogin();
	return res;
}

function IsAuthenticated() {
    if (ServerParams.getParam("token") != "") {
        return 1;
    }
    return 0;
}

function DoLogout() {
    local token = ServerParams.getParam("token");
    if (token == "" ) {
        return 0;
    }
    local url = "https://api.dropboxapi.com/2/auth/token/revoke";
    nm.setUrl(url);
    signRequest(url, token);
    nm.addQueryHeader("Content-Type", "application/json");
    nm.doPost("null");
    ServerParams.setParam("token", "");
    
    if (nm.responseCode() == 200) {
        return 1;
    } else {
        local t = ParseJSON(nm.responseBody());
        if ("error" in t && t.error.rawget(".tag") == "invalid_access_token") {
            WriteLog("warning", "[dropbox.nut] Token already revoked.");
            return 1;
        }
    }
    return 0;
}

function min(a,b) {
	return a < b ? a : b;
}

function  UploadFile(FileName, options) {		
	if (!DoLogin() ) {
		return 0;
	}
	local token =  ServerParams.getParam("token");
	if (!RefreshToken()) {
		return 0;
	}
	local url = null;
	local userPath = ServerParams.getParam("UploadPath");
	if ( userPath!="" && userPath[userPath.len()-1] != "/") {
		userPath+= "/";
	}
	local chunkSize = (50*1024*1024).tofloat();
	local fileSize = 0;
	try { 
		fileSize=GetFileSize(FileName);
	} catch ( ex ) {
		
	}
	
	if ( fileSize < 0 ) {
		WriteLog("error", "[dropbox.nut] fileSize < 0 ");
		return 0;
	}
	local path = "/"+ userPath;
    local remotePath =path+ExtractFileName(FileName);
    local fileId="";
	if ( fileSize > 150000000 ) {
		local chunkCount = ceil(fileSize / chunkSize);
		local session = null;
		local offset = 0;
        
        local session="";
		for(local i = 0; i < chunkCount; i++ ) {
			for ( local j =0; j < 2; j++ ) {
				try {
					nm.setChunkOffset(offset.tofloat());
				} catch ( ex ) {
					WriteLog("error", "Your Image Uploader version does not support chunked uploads for big files. \r\nPlease update to the latest version");
					return 0;
				}
				if( session==""){
                    url = "https://content.dropboxapi.com/2/files/upload_session/start" ;
                    signRequest(url, token);
                    local arg ={
                        close=false
                    };
                    local json = reg_replace(ToJSON(arg),"\n","");
                    nm.addQueryHeader("Dropbox-API-Arg", json);
                } else{
                    url = "https://content.dropboxapi.com/2/files/upload_session/append_v2" ;
                    signRequest(url, token);
                    local arg ={
                        cursor={
                            session_id=session,
                            offset=offset
                        },
                        close=false
                    };
                    local json = reg_replace(ToJSON(arg),"\n","");
                    nm.addQueryHeader("Dropbox-API-Arg", json);
                }
				local chunkSize = min(chunkSize,fileSize.tofloat()-offset).tointeger();
				nm.setChunkSize(chunkSize);
                nm.addQueryHeader("Content-Type", "application/octet-stream");
				nm.setUrl(url);
				nm.doUpload(FileName,"");
                
				if ( nm.responseCode() != 200 ) {
					WriteLog("warning", "[dropbox.nut] Chunk upload failed, offset="+offset+", size="+chunkSize+(j< 1? "Trying again..." : ""));
					if ( nm.responseCode() == 403 ) {
						WriteLog("error", "[dropbox.nut] Upload failed. Access denied");
						return 0;
					}
				} else {
					local t = ParseJSON(nm.responseBody());
					if(session==""){
                        session = t.session_id;
                    }else{
                        
                    }
                    offset += chunkSize;
                    
					break;
				}
			}
			//return 0;
		}
		if ( session == "" ) {
			WriteLog("error", "[dropbox.nut] Upload failed");
			return 0;
		}
		url = "https://content.dropboxapi.com/2/files/upload_session/finish";
		
		nm.setUrl(url);
        local arg ={
            cursor={
                session_id=session,
                offset=offset
            },
            commit={
                path=remotePath,
                mode="add",
                autorename=true,
                mute=false
            }
        };
        local json = reg_replace(ToJSON(arg),"\n","");
        nm.addQueryHeader("Dropbox-API-Arg", json);
        nm.addQueryHeader("Content-Type", "application/octet-stream");
		signRequest(url, token);
		nm.setMethod("POST");
		nm.doPost("");

		if ( nm.responseCode() != 200 ) {
            return 0;
		}
        local data = ParseJSON(nm.responseBody());

	} else {
		url = "https://content.dropboxapi.com/2/files/upload" ;
		signRequest(url, token);
        local arg ={
            path=remotePath,
            mode="add",
            autorename=true,
            mute=false
        };
        nm.addQueryHeader("Content-Type", "application/octet-stream");
        local json = reg_replace(ToJSON(arg),"\n","");
        nm.addQueryHeader("Dropbox-API-Arg", json);
		nm.setUrl(url);
		nm.doUpload(FileName, "");
	}

    local data = ParseJSON(nm.responseBody());
    if(nm.responseCode()!=200){
        WriteLog("error", "[dropbox.nut] " + nm.responseBody());
        return 0;
    }

    fileId = data.id;
    
    url = "https://api.dropboxapi.com/2/sharing/create_shared_link_with_settings" ;
	signRequest(url, token);
    local arg ={
            path=remotePath,
            settings={
                requested_visibility="public"
            }
    };
    local json = reg_replace(ToJSON(arg),"\n","");
    nm.addQueryHeader("Content-Type","application/json")
	nm.setUrl(url);
    nm.enableResponseCodeChecking(false);
	nm.doPost(json);
    
    local viewUrl = "";
    
    if(nm.responseCode()!=200){ 
        if (nm.responseCode() == 409) { // Shared link already exists
            data = ParseJSON(nm.responseBody());
            url = "https://api.dropboxapi.com/2/sharing/list_shared_links" ;
            signRequest(url, token);
            local arg ={
                    path=remotePath,
                    direct_only=true
            };
            local json = reg_replace(ToJSON(arg),"\n","");
            nm.addQueryHeader("Content-Type","application/json")
            nm.setUrl(url);
            nm.doUpload("",json);
            if (nm.responseCode() == 200) {
                data = ParseJSON(nm.responseBody());
                if ("links" in data && data.links.len() > 0){
                    viewUrl = data.links[0].url;
                    options.setViewUrl( viewUrl );
                }
            } else {
                WriteLog("error", "[dropbox.nut] " + nm.responseBody());
                return 0;
            }
        } else {
            WriteLog("error", "[dropbox.nut] " + nm.responseBody());
            return 0;
        }
       
    } else {
        data = ParseJSON(nm.responseBody());
        viewUrl = data.url;
        options.setViewUrl( viewUrl);
    }
	
	if ( viewUrl != "" ) {
		return 1;
	}
 	
	return 0;
}

function reg_replace(str, pattern, replace_with)
{
	local resultStr = str;	
	local res;
	local start = 0;

	while( (res = resultStr.find(pattern,start)) != null ) {	

		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
		start = res + replace_with.len();
	}
	return resultStr;
}

function GetServerParamList()
{
	return {
        token = "token"
		UploadPath = "Upload Path"
	};
}