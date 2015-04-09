
function regex_simple(data,regStr,start)
{
	local ex = regexp(regStr);
	local res = ex.capture(data, start);
	local resultStr = "";
	if(res != null){	
		resultStr = data.slice(res[1].begin, res[1].end);
	}
		return resultStr;
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
	} else if ( /*nm.responseCode() == 0 ||*/ (nm.responseCode() >= 400 && nm.responseCode() <= 499)) {
		WriteLog("error", "Response code " + nm.responseCode() + "\r\n" + nm.errorString() );
		return 0;
	}
	return 1;
}

function DoLogin() 
{ 
	local login = ServerParams.getParam("Login");
	local scope = "https://www.googleapis.com/auth/drive";
	local redirectUrl = "urn:ietf:wg:oauth:2.0:oob";
	local clientSecret = "65ie-G5nWqGMv_THtY3z2snZ";
	local clientId = "162038470312-dn0kut9j7l0cd9lt32r09j0c841goei9.apps.googleusercontent.com";

	if(login == "" ) {
		WriteLog("error", "E-mail should not be empty!");
		return 0;
	}
	
	local token = ServerParams.getParam("token");
	local tokenType = ServerParams.getParam("tokenType");
	if ( token != "" && tokenType != "" &&  ServerParams.getParam("prevLogin") == login ) {
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
				local data =  nm.responseBody();
				token = regex_simple(data, "access_token\": \"(.+)\"", 0);
				ServerParams.setParam("expiresIn", regex_simple(data, "expires_in\": (\\d+)", 0));
				tokenType = regex_simple(data, "token_type\": \"(.+)\"", 0);
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
			}
		} else {
			return 1;
		}
	}

	local url = "https://accounts.google.com/o/oauth2/auth?scope="+ nm.urlEncode(scope) +"&redirect_uri="+redirectUrl+"&response_type=code&"+ "client_id="+clientId;
	ShellOpenUrl(url);
	
	local confirmCode = InputDialog(tr("googledrive.confirmation.text", "You need to need to sign in to your Google account in web browser which just have opened and then copy confirmation code into the text field below. Please enter confirmation code:"),"");
	
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
	local data =  nm.responseBody();
	local accessToken = regex_simple(data, "access_token\": \"(.+)\"", 0);
	local timestamp = time();
	if ( accessToken != "" ) {
		token = accessToken;
		
		ServerParams.setParam("token", token);
		ServerParams.setParam("expiresIn", regex_simple(data, "expires_in\": (\\d+)", 0));
		ServerParams.setParam("refreshToken", regex_simple(data, "refresh_token\": \"(.+)\"", 0));
		tokenType = regex_simple(data, "token_type\": \"(.+)\"", 0);
		//p;
		ServerParams.setParam("tokenType", tokenType);
		ServerParams.setParam("prevLogin", login);
		ServerParams.setParam("tokenTime", ""+timestamp);
		return 1;
	}	else {
		WriteLog("error", "Autentication failed");
	}
	return 0;		
} 

function GetFolderList(list)
{
	if(!DoLogin())
		return 0;
	nm.addQueryHeader("Authorization", getAuthorizationString());
	nm.doGet("https://www.googleapis.com/drive/v2/files");
	if ( nm.responseCode() == 200 ) {
		//DebugMessage(nm.responseBody(), true);
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

function CreateFolder(parentFolder, folder)
{
	if(!DoLogin()) {
		return 0;
	}

	nm.setUrl("https://www.googleapis.com/drive/v2/files");
	nm.addQueryHeader("Authorization", getAuthorizationString());
	
	local data = {
		title = folder.getTitle(),
		mimeType = "application/vnd.google-apps.folder"
	};
	nm.addQueryHeader("Content-Type","application/json");
	nm.doPost(ToJSON(data));
	if ( checkResponse() ) {
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

function ModifyFolder(folder)
{
	if(!DoLogin())
		return 0;
	
	local title = folder.getTitle();
	local id = folder.getId();

	nm.setMethod("PUT");
	nm.setUrl("https://www.googleapis.com/drive/v2/files/" + id);
	nm.addQueryHeader("Authorization", getAuthorizationString());
	nm.addQueryHeader("Content-Type", "application/json");
	local postData = {
		title = title,
	};
	nm.doUpload("", ToJSON(postData));
	if ( checkResponse() ) {
		return 1;
	}

	return 0;
}
	

function  UploadFile(FileName, options)
{
	if(!DoLogin())
		return -1;
	
	/*nm.addQueryHeader("Authorization", getAuthorizationString()); // just for fiddler
	nm.doGet("https://www.googleapis.com/");*/
	
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
	if ( folderId != "" ) {
		postData.parents = [ {id = folderId, kind = "drive#parentReference"}];
	}
	local str = ToJSON(postData);
	
	nm.setUrl("https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable");
	nm.addQueryHeader("Authorization", getAuthorizationString());
	nm.setMethod("POST");
	nm.doUpload("", str);
	if ( checkResponse() ) {
		local sessionUri = nm.responseHeaderByName("Location");
		if ( sessionUri != "" ) {
			nm.setMethod("PUT");
			nm.addQueryHeader("Authorization", getAuthorizationString());
			nm.addQueryHeader("Content-Type", mimeType);
			nm.setUrl(sessionUri);
			nm.doUpload(FileName, "");
			if ( checkResponse() ) {
					local responseData = nm.responseBody();
					local item = ParseJSON(responseData);
					nm.addQueryHeader("Authorization", getAuthorizationString());
					nm.setUrl("https://www.googleapis.com/drive/v2/files/"+ item.id + "/permissions");
					local postData = {
						role = "reader",
						type = "anyone", 
					};
					nm.addQueryHeader("Content-Type", "application/json");
					nm.setMethod("POST");
					nm.doUpload("", ToJSON(postData));
					options.setViewUrl(item.alternateLink);
					try {
						options.setThumbnailUrl(item.thumbnailLink);
					} catch ( ex ) {
						
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
