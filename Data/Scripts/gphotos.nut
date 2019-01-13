login <- ""; 

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

function getAuthorizationString() {
	local token = ServerParams.getParam("token");
	local tokenType = ServerParams.getParam("tokenType");
	return tokenType + " " + token ;
}

function _DoLogin() 
{ 
	local login = ServerParams.getParam("Login");
	local scope = "https://www.googleapis.com/auth/photoslibrary https://www.googleapis.com/auth/photoslibrary.sharing";
	local redirectUrl = "urn:ietf:wg:oauth:2.0:oob";
	local clientSecret = "49GuU_mFjyY-9zjCNB3E0FV7";
	local clientId = "327179857936-lcpkeaoidkl5cru001tpvv2mudi5ok7g.apps.googleusercontent.com";

	if(login == "" ) {
		WriteLog("error", "E-mail should not be empty!");
		return 0;
	}
	
	local token = ServerParams.getParam("token");
	local tokenType = ServerParams.getParam("tokenType");
	if ( token != "" && tokenType != "") {
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
			// Refresh access token
			nm.setUrl("https://www.googleapis.com/oauth2/v3/token");
			nm.addQueryParam("refresh_token", refreshToken); 
			nm.addQueryParam("client_id", clientId); 
			nm.addQueryParam("client_secret", clientSecret); 
			nm.addQueryParam("grant_type", "refresh_token"); 
			nm.doPost("");
			if ( checkResponse() ) {
				local parsedData = ParseJSON(nm.responseBody());
                if ("access_token" in parsedData) {
                    token = parsedData.access_token;
                }
				ServerParams.setParam("expiresIn", "" + parsedData.expires_in);
				tokenType = parsedData.token_type;
				ServerParams.setParam("tokenType", tokenType);
				ServerParams.setParam("token", token);
				ServerParams.setParam("tokenTime", time().tostring());
				if ( token != "" ) {
					return 1;
				} else {
					token = "";
					tokenType = "";
					return 0;
				}
			} else {
                return 0; //<-- need to check this
            }
		} else {
			return 1;
		}
	}

	local url = "https://accounts.google.com/o/oauth2/auth?scope="+ nm.urlEncode(scope) +"&redirect_uri="+redirectUrl+"&response_type=code&"+ "client_id="+clientId;
	ShellOpenUrl(url);
	
	local confirmCode = InputDialog("You need to need to sign in to your Google account in web browser which just has been opened and then copy confirmation code into the text field below. Please enter the confirmation code:", "");
	
	if ( confirmCode == "" ) {
		print("Cannot authenticate without confirm code");
		return 0;
	}
	
	nm.setUrl("https://www.googleapis.com/oauth2/v3/token");
	nm.addQueryParam("code", confirmCode); 
	nm.addQueryParam("client_id", clientId); 
	nm.addQueryParam("client_secret", clientSecret); 
	nm.addQueryParam("redirect_uri", redirectUrl); 
	nm.addQueryParam("grant_type", "authorization_code"); 
	nm.doPost("");
	if ( !checkResponse() ) {
		return 0;
	}
	local parsedResponse = ParseJSON(nm.responseBody());
	local accessToken = "";
    if ("access_token" in parsedResponse) {
        accessToken = parsedResponse.access_token;
    }
	local timestamp = time();
	if ( accessToken != "" ) {
		token = accessToken;
		
		ServerParams.setParam("token", token);
		ServerParams.setParam("expiresIn", "" + parsedResponse.expires_in);
		ServerParams.setParam("refreshToken", parsedResponse.refresh_token);
		tokenType = parsedResponse.token_type;
		
		ServerParams.setParam("tokenType", tokenType);
		ServerParams.setParam("tokenTime", ""+timestamp);
		return 1;
	}	else {
		WriteLog("error", "Autentication failed");
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

function internal_parseAlbumList(data,list)
{
    local t = ParseJSON(data);
    if ( t != null ) {
        local count = t.albums.len();
        for ( local i = 0; i < count; i++ ) {
            local item = t.albums[i];
                    
            local folder = CFolderItem();
            folder.setId(item.id);
            folder.setTitle(item.title);
            //folder.setSummary(summary);
            folder.setViewUrl(item.productUrl);
            list.AddFolderItem(folder);
		}
		return 1;
	}

	return 0;
}

function checkResponse() {
	if ( nm.responseCode() == 403 ) {
		if ( nm.responseBody().find("Invalid token",0)!= null) {
			WriteLog("warning", nm.responseBody());
			ServerParams.setParam("token", "");
			ServerParams.setParam("expiresIn", "");
			ServerParams.setParam("refreshToken", "");
			ServerParams.setParam("tokenType", "");
			ServerParams.setParam("prevLogin", "");
			ServerParams.setParam("tokenTime", "");
			return 1 + DoLogin();
		} else {
			WriteLog("error", "403 Access denied" );
			return 0;
		}
	} else if ( nm.responseCode() == 0 || (nm.responseCode() >= 400 && nm.responseCode() <= 499)) {
		WriteLog("error", "Response code " + nm.responseCode() + "\r\n" + nm.errorString() );
		return 0;
	}
	return 1;
}

function internal_loadAlbumList(list)
{
	local i = 0;
	while ( 1 ) {
		i++;
		nm.setUrl("https://photoslibrary.googleapis.com/v1/albums");
		nm.addQueryHeader("Expect","");
		nm.addQueryHeader("Authorization", getAuthorizationString());
		nm.doGet("");
		local response = checkResponse();
		if ( response == 0 ) {
			return 0;
		} else if ( response == 1 ) {
			break;
		} else if (  i > 3 ) {
			return 0;
		}
	}
	
	return internal_parseAlbumList(nm.responseBody(),list);	
}

function GetFolderList(list)
{
	if(!DoLogin())
		return 0;
	
	return internal_loadAlbumList(list);
}

function CreateFolder(parentAlbum,album)
{
    if(!DoLogin()) {
        return 0;
    }
	
    nm.addQueryHeader("Expect", "");
    nm.setUrl("https://photoslibrary.googleapis.com/v1/albums");
    nm.addQueryHeader("Authorization", getAuthorizationString());
	
    local data = {
        album = {
            title = album.getTitle()
        }
    };
    nm.addQueryHeader("Content-Type","application/json");
    nm.doPost(ToJSON(data));
    if ( nm.responseCode() == 200) {
        local t = ParseJSON(nm.responseBody())
        album.setId(t.id);
        //album.setViewUrl(t.productUrl);
    }
    
	return 1;
}

function UploadFile(FileName, options)
{
	if(!DoLogin()) {
		return -1;
    }
		
	local i = 0;
	while ( i < 3 ) {
		i++;
		local albumStr = options.getFolderID();
        if (albumStr == "") {
            WriteLog("error", "[Google Photos] You should select an album before upload");
            return -1;
        }
		nm.setUrl("https://photoslibrary.googleapis.com/v1/uploads");
		nm.addQueryHeader("Authorization", getAuthorizationString());

		local ServerFileName = options.getServerFileName();
		if(ServerFileName=="") ServerFileName = ExtractFileName(FileName);
		local encodedFname = /*nm.urlEncode*/reg_replace(ServerFileName, " ", "_");
		nm.addQueryHeader("X-Goog-Upload-File-Name", encodedFname);
		nm.addQueryHeader("X-Goog-Upload-Protocol", "raw");
		nm.addQueryHeader("Expect","");
		nm.addQueryHeader("Content-Type", "application/octet-stream");
		nm.doUpload(FileName, "");
		
		local responseInfo = checkResponse ();
		if ( responseInfo == 0 ) {
			return 0;
		} else if ( responseInfo == 2 ) {
			continue;
		}

		local uploadToken = nm.responseBody();
        
        nm.setUrl("https://photoslibrary.googleapis.com/v1/mediaItems:batchCreate");
        nm.addQueryHeader("Authorization", getAuthorizationString());
        nm.addQueryHeader("Content-Type", "application/json");
        local requestData = {
                newMediaItems = [
                    {
                        description = "",
                        simpleMediaItem = {
                            uploadToken = uploadToken
                        }
                    }
                ]
        };
        if (albumStr != "") {
            requestData.albumId <- albumStr;
        }
        nm.doPost(ToJSON(requestData));
        if (nm.responseCode() == 200) {
            local t = ParseJSON(nm.responseBody());
            if( "newMediaItemResults" in t && t.newMediaItemResults.len() > 0) {
                local item = t.newMediaItemResults[0];
                if (albumStr != "") {
                    local shareUrl = Sync.getValue("shareUrl");
                    if (shareUrl=="") {
                        nm.addQueryHeader("Authorization", getAuthorizationString());
                        nm.doGet("https://photoslibrary.googleapis.com/v1/albums/" + albumStr);
                        if (nm.responseCode() == 200) {
                            local album = ParseJSON(nm.responseBody());
                            if ("shareInfo" in album) {
                                shareUrl = album.shareInfo.shareableUrl;
                            }
                        }
                        
                        if (shareUrl == "") {
                            nm.setUrl("https://photoslibrary.googleapis.com/v1/albums/" + albumStr + ":share");
                            nm.addQueryHeader("Content-Type", "application/json");
                            local postData = {
                                sharedAlbumOptions = { 
                                }
                            };
                            nm.addQueryHeader("Authorization", getAuthorizationString());
                            nm.doPost(ToJSON(postData));
                            if (nm.responseCode() == 200) {
                                local parsedData = ParseJSON(nm.responseBody());
                                //options.setDirectUrl(directUrl);
                                //options.setThumbUrl(thumbUrl);
                                shareUrl = parsedData.shareInfo.shareableUrl;
                            }
                        }
                        if ( shareUrl != "" ) {
                            Sync.setValue("shareUrl", shareUrl);
                        }
                    }
                    options.setViewUrl(shareUrl);
                }
                return 1;
            }  
        }
	}
	
	return 0;
}

function ModifyFolder(album)
{
    return 0;
}

function GetFolderAccessTypeList()
{
	local a=["Private", "Public"];
	return a;
}

function GetServerParamList()
{
	local a =
	{
		token = "token",
		tokenType = "tokenType",
		expiresIn = "expiresIn",
		refreshToken = "refreshToken",
		tokenTime = "tokenTime"
	}
	return a;
}