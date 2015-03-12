login <- ""; 

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

function _WriteLog(type,message) {
	try {
		WriteLog(type, message);
	} catch (ex ) {
		print(type + " : " + message);
	}
}

function inputBox(prompt, title) {
	try {
		return InputDialog(prompt, "");
	}catch (e){}
	local tempScript = "%temp%\\imguploader_inputbox.vbs";
	prompt = reg_replace(prompt, "\n", "\" ^& vbCrLf ^& \"" );
	local tempOutput = getenv("TEMP") + "\\imguploader_inputbox_output.txt";
	local command = "echo result = InputBox(\""+ prompt + "\", \""+ title + "\") : Set objFSO=CreateObject(\"Scripting.FileSystemObject\") : Set objFile = objFSO.CreateTextFile(\"" + tempOutput + "\",True) : objFile.Write result : objFile.Close  > \"" + tempScript + "\"";
	system(command);
	command = "cscript /nologo \"" + tempScript + "\"";// > \"" + tempOutput + "\"";*/
	system(command);
	local res = readFile(tempOutput);
	system("rm \""+ tempOutput + "\"");
	return res;
}

function openUrl(url) {
	try{
		return ShellOpenUrl(url);
	}catch(ex){}

	system("start "+ reg_replace(url,"&","^&") );
}

function getAuthorizationString() {
	local token = ServerParams.getParam("token");
	local tokenType = ServerParams.getParam("tokenType");
	return tokenType + " " + token ;
}

function DoLogin() 
{ 
	local login = ServerParams.getParam("Login");
	local scope = "https://picasaweb.google.com/data/";
	local redirectUrl = "urn:ietf:wg:oauth:2.0:oob";
	local clientSecret = "65ie-G5nWqGMv_THtY3z2snZ";
	local clientId = "162038470312-dn0kut9j7l0cd9lt32r09j0c841goei9.apps.googleusercontent.com";

	if(login == "" ) {
		_WriteLog("error", "E-mail should not be empty!");
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
	openUrl(url);
	
	local confirmCode = inputBox("You need to need to sign in to your Google Picasa Web Albums account in web browser which just have opened and then copy confirmation code into the text field below. Please enter confirmation code:", "Image Uploader - Enter confirmation code");
	
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
		_WriteLog("error", "Autentication failed");
	}
	return 0;		
} 

function internal_parseAlbumList(data,list)
{
	local start = 0;
	
	while(1)
	{
		local title,id,summary="";
		local ex = regexp("<entry");
		local res = ex.search(data, start);
		local link = "";
		local access = "private";
		
		local album = CFolderItem();
		if(res != null){	
			start = res.end;
		}
		else break;
		
		id = regex_simple(data, "<gphoto:id>(\\d+)</gphoto:id>", start);
		title = regex_simple(data, "<title.+>(.+)</title>", start);
		summary = regex_simple(data, "<summary.+>(.+)</summary>", start);
		access = regex_simple(data, "<gphoto:access>(.+)</gphoto:access>", start);
		
		if( access == "private")
			album.setAccessType(0);
		else
			album.setAccessType(1);
		
		link = regex_simple(data, "type=[\",']text/html[\",'] href=[\",'](.+)[\",']", start);
		
		album.setId(id);
		album.setTitle(title);
		album.setSummary(summary);
		album.setViewUrl(link);
		list.AddFolderItem(album);
		
	}
	return 1;
}

function checkResponse() {
	if ( nm.responseCode() == 403 ) {
		if ( nm.responseBody().find("Invalid token",0)!= null) {
			_WriteLog("warning", nm.responseBody());
			ServerParams.setParam("token", "");
			ServerParams.setParam("expiresIn", "");
			ServerParams.setParam("refreshToken", "");
			ServerParams.setParam("tokenType", "");
			ServerParams.setParam("prevLogin", "");
			ServerParams.setParam("tokenTime", "");
			return 1 + DoLogin();
		} else {
			_WriteLog("error", "403 Access denied" );
			return 0;
		}
	} else if ( nm.responseCode() == 0 || (nm.responseCode() >= 400 && nm.responseCode() <= 499)) {
		_WriteLog("error", "Response code " + nm.responseCode() + "\r\n" + nm.errorString() );
		return 0;
	}
	return 1;
}

function internal_loadAlbumList(list)
{
	local i = 0;
	while ( 1 ) {
		i++;
		nm.setUrl("https://picasaweb.google.com/data/feed/api/user/default");
		nm.addQueryHeader("Expect","");
		nm.addQueryParam("GData-Version", "2");
		nm.addQueryHeader("Authorization", getAuthorizationString());
		nm.doGet("");
		local response = checkResponse ();
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
	local title =album.getTitle();
	local summary = album.getSummary();
	local accessType = album.getAccessType();
	local parentId = album.getParentId; 
	local strAcessType = "private";

	if(!DoLogin())
			return 0;
	
	if (accessType == 1)
		strAcessType = "public";
	
	nm.addQueryHeader("Expect","");
	nm.addQueryHeader("GData-Version", "2");
	nm.setUrl("https://picasaweb.google.com/data/feed/api/user/default");
	nm.addQueryHeader("Authorization", getAuthorizationString());
	
	local data = "<entry xmlns='http://www.w3.org/2005/Atom'"+
		" xmlns:media='http://search.yahoo.com/mrss/'"+
		" xmlns:gphoto='http://schemas.google.com/photos/2007'>"+
		"<title type='text'>"+title+"</title>"+
		" <summary type='text'>"+summary+"</summary>"+
		"<gphoto:access>"+strAcessType+"</gphoto:access>"+
		"<category scheme='http://schemas.google.com/g/2005#kind'"+
		" term='http://schemas.google.com/photos/2007#album'></category>"+
		"</entry>";
	nm.addQueryHeader("Content-Type","application/atom+xml");
	nm.doPost(data);
	local response_data = nm.responseBody();
	 
	local id = regex_simple(response_data, "<gphoto:id>(\\d+)</gphoto:id>", 0);
	album.setId(id);
	
	local link = regex_simple(response_data, "type=[\",']text/html[\",'] href=[\",'](.+)[\",']", 0);
	album.setViewUrl(link);
	
	return 1;
}

function  UploadFile(FileName, options)
{
	if(!DoLogin())
		return -1;
		
	local i = 0;
	while ( i < 3 ) {
		i++;
		nm.addQueryParam("GData-Version", "2");
		local albumID = "default";
		local albumStr = options.getFolderID();
		if(albumStr!="")
			albumID = albumStr;
		
		nm.setUrl("https://picasaweb.google.com/data/feed/api/user/default/albumid/"+albumID);
		nm.addQueryHeader("Authorization", getAuthorizationString());

		local ServerFileName = options.getServerFileName();
		if(ServerFileName=="") ServerFileName = ExtractFileName(FileName);
		local 	encodedFname = nm.urlEncode(ServerFileName);
		nm.addQueryHeader("Slug",encodedFname);
		nm.addQueryHeader("Expect","");
		nm.addQueryHeader("Content-Type",GetFileMimeType(FileName));
		nm.doUpload(FileName,"");
		
		local responseInfo = checkResponse ();
		if ( responseInfo == 0 ) {
			return 0;
		} else if ( responseInfo == 2 ) {
			continue;
		}

		local data = nm.responseBody();

		
		local directUrl = regex_simple(data, "content type='image/.+src='(.+)'", 0);
		
		local slashRegexp = regexp( "/"  ); 
		local slashPos = slashRegexp.search( directUrl);
		local newPos = slashPos;
		while ( true ) {
			newPos = slashRegexp.search( directUrl, newPos.begin + 1 ); 
			if ( newPos == null) {
				break;
			}
			slashPos = newPos;
		}	
		
		if ( slashPos != null ) {
			directUrl =  directUrl.slice( 0, slashPos.begin)+ "/s0/" + directUrl.slice(  slashPos.begin+1);
		}
		local thumbUrl = regex_simple(data, "<media:thumbnail.+<media:thumbnail.+<media:thumbnail url='(.+)'", 0);
		
		local viewUrl = 
			regex_simple(data, "<link rel=[\",']http://schemas.google.com/photos/2007#canonical[\",'] type=[\",']text/html[\",'] href='(.+)'", 0);

		options.setDirectUrl(directUrl);
		options.setThumbUrl(thumbUrl);
		options.setViewUrl(viewUrl);
		return 1;
	}
	
	return 0;
}

function ModifyFolder(album)
{
	if(!DoLogin())
		return 0;
	
	local title =album.getTitle();
	local id = album.getId();
	local summary = album.getSummary();
	local accessType = album.getAccessType();
	local parentId = album.getParentId; 
	local strAcessType = "private";
	
	if (accessType == 1)
		strAcessType = "public";

	nm.addQueryHeader("Expect","");
	nm.setMethod("PATCH");
	nm.addQueryHeader("GData-Version", "2");
		nm.addQueryHeader("If-Match","*");
	nm.setUrl("https://picasaweb.google.com/data/entry/api/user/default/albumid/"+id);
	nm.addQueryHeader("Authorization", getAuthorizationString());
	
	 local data = "<entry xmlns='http://www.w3.org/2005/Atom'"+
	   " xmlns:media='http://search.yahoo.com/mrss/'"+
	   " xmlns:gphoto='http://schemas.google.com/photos/2007'>"+
	  "<title type='text'>"+title+"</title>"+
	 " <summary type='text'>"+summary+"</summary>"+
	 "<gphoto:access>"+strAcessType+"</gphoto:access>"+
	  "<category scheme='http://schemas.google.com/g/2005#kind'"+
	   " term='http://schemas.google.com/photos/2007#album'></category>"+
	"</entry>";
	 nm.addQueryHeader("Content-Type","application/xml");
	
	 nm.doUpload("", data);
	 return 1;
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
		prevLogin = "prevLogin",
		tokenTime = "tokenTime"
	}
	return a;
}