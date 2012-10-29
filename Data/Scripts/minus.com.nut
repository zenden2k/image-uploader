ConsumerKey <- "b57b69843f7a302a276dde89890fc6";
ConsumerSecret <- "3fb097f08314d713959b1f41d543b0";
AccessToken <-"";
UserName <- "";
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

function internal_createFolder(name, isPublic) {
	nm.setUrl(getUserFolderUrl());
	nm.addQueryParam("name", name);
	nm.addQueryParam("is_public", isPublic?"true":"false");
	nm.doPost("");
	local data = nm.responseBody();
	local url =  regex_simple(data, "files\": \"(.+)\"", 0);
	return regex_simple(url, "/folders/(.+)/files", 0);
}

function CreateFolder(parentAlbum,album) {
	local title =album.getTitle();
	local summary = album.getSummary();
	local accessType = album.getAccessType();
	local parentId = album.getParentId; 
	
	if ( AccessToken == "" ) {
		if ( !Authorize() ) {
			return 0;
		}
	}
	
	local id = internal_createFolder( title, accessType == 0 );
	album.setId(id);
	album.setViewUrl("http://min.us/m" + id);
	return 1;
}

function getAccessToken() {
	local username = ServerParams.getParam("Login");
	local password =  ServerParams.getParam("Password");
	
	local url = "https://minus.com//oauth/token?grant_type=password&client_id="+ ConsumerKey + "&client_secret="+ ConsumerSecret +
		"&scope=upload_new&username=" + username + "&password=" + password;
	nm.doGet(url);
	local data = nm.responseBody();
	local accessToken = regex_simple(data, "access_token\": \"(.+)\"",0);

	return accessToken;
}

function getFolderId(){
	nm.doGet(getUserFolderUrl());
}

function getUserFolderUrl(){
	return "https://minus.com/api/v2//users/" + UserName + "/folders?bearer_token=" + AccessToken ;
}

function Authorize() {
	AccessToken <- getAccessToken();
	UserName <- ServerParams.getParam("Login");
	return true;
}

function GetFolderList(list) {
	if ( AccessToken == "" ) {
		Authorize();
	}

	nm.doGet(getUserFolderUrl());
	local data = nm.responseBody();

	internal_parseFolderList(data, list);
	return 1; //success
}

function CreateOrUseExistingFolder(name) {	
	local start = 0;
	nm.doGet(getUserFolderUrl());
	local data = nm.responseBody();

	while(1)
	{
		local title="",id="";
		local ex = regexp("\"files\":");
		local res = ex.search(data, start);
		
		if(res != null){	
			start = res.end;
		}
		else {
			break;
		}
		
		id = regex_simple(data,"\"id\": \"([^\"]+)",start);
		title = regex_simple(data,"\"name\": \"([^\"]+)\"",start);
		if ( title == "ImageUploader") {
			return id;
		}
	
	}
	return internal_createFolder("ImageUploader", false);
}

function internal_parseFolderList(data,list) {
	local start = 0;
	
	while(1)
	{
		local title="",id="";
		local ex = regexp("\"files\":");
		local res = ex.search(data, start);
		local isPublic = "";
		local link = "";
		local album = CFolderItem();
		if(res != null){	
			start = res.end;
		}
		else break;
		
		id = regex_simple(data,"\"id\": \"([^\"]+)",start);
		title = regex_simple(data,"\"name\": \"([^\"]+)\"",start);
		link = "http://min.us/m" + id;
		isPublic = regex_simple(data,"\"is_public\": (\\d+)",start);
		album.setId(id);
		album.setTitle(title);
		album.setViewUrl(link);
		album.setAccessType(isPublic=="0"? 1: 0);
		list.AddFolderItem(album);
		
	}
}

function  UploadFile(FileName, options) {	

	local login = ServerParams.getParam("Login");
	UserName <- login;
	local password =  ServerParams.getParam("Password");
	accessToken <- getAccessToken();
	local folderId =  options.getFolderID();
	
	if ( folderId == "" ) {
		folderId = CreateOrUseExistingFolder("ImageUploader");
	}
	
	local uploadUrl = "https://minus.com/api/v2/folders/" + folderId + "/files?bearer_token=" + accessToken;
	local fileBaseName = ExtractFileName(FileName);
	nm.addQueryParam("caption", fileBaseName);
	nm.addQueryParam("filename", fileBaseName); 
	nm.addQueryParamFile("file",FileName, ExtractFileName(FileName),"");
	nm.setUrl(uploadUrl);
	
	nm.doUploadMultipartData();
 	local data = nm.responseBody();
	
	local message = regex_simple(data, "message\": \"(.+)\"", 0);
	local directUrl = regex_simple(data, "url_rawfile\": \"(.+)\"", 0);
	
	local  fileId = regex_simple(data, "id\": \"(.+)\"", 0);
	if ( fileId == "" || fileId == null ) {
		print("Error: " + message);
		return 0;	
	}
	local  thumbUrl = regex_simple(data, "url_thumbnail\": \"(.+)\"", 0);
	
	local ext = GetFileExtension(directUrl);
	if ( ext == "png" || ext == "jpg" || ext == "gif" ){
		directUrl = thumbUrl;
	}

	local viewUrl = "http://min.us/l" + fileId;
	local thumbs = regex_simple(data, "url_thumbnails\": \\{(.+)\\}", 0);
		
	local thumbnail_small = regex_simple(thumbs, "\\d+\": \"(http://[^/]+/[^\"]+_ss.[^\"]+)\"", 0);

	
	if ( thumbnail_small != "" ) {
		thumbUrl = thumbnail_small;
	}
	if(directUrl == "")
		return 0;
	options.setDirectUrl(directUrl);
	options.setViewUrl(viewUrl);
	options.setThumbUrl(thumbUrl);	
	return 1;
}

function GetFolderAccessTypeList()
{
	local a=["Public","Private"];
	return a;
}