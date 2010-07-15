token <- ""; 

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
function doLogin() 
{ 
	local email = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");

	if(email == "" || pass=="")
	{
		print("E-mail and password should not be empty!");
		return 0;
	}
	nm.addQueryHeader("Expect","");
	nm.setUrl("https://www.google.com/accounts/ClientLogin");
  
	nm.addQueryParam("accountType", "HOSTED_OR_GOOGLE"); 

	nm.addQueryParam("Email", email); 
	nm.addQueryParam("Passwd", pass); 
	nm.addQueryHeader("Expect", "");
   nm.addQueryParam("service", "lh2"); 
  	nm.addQueryParam("source", "zendendotws-imageuploader-1.2"); 
   nm.doPost("");
   local data =  nm.responseBody();
  
  
   local ex = regexp("Auth=(\\S+)");
	local res = ex.capture(data);
	if(res != null){	
		token = data.slice(res[1].begin,res[1].end);
	}
	else {
	print("Eror while authencation using username "+email);
		return 0;
	}
	return 1; //Success login
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
}

function internal_loadAlbumList(list)
{
	nm.setUrl("http://picasaweb.google.com/data/feed/api/user/default");
  	nm.addQueryHeader("Expect","");
	nm.addQueryParam("GData-Version", "2");
	nm.addQueryHeader("Authorization","GoogleLogin auth=" + token);
	nm.doGet("");
	internal_parseAlbumList(nm.responseBody(),list);
}

function GetFolderList(list)
{
	if(token == "")
	{
		if(!doLogin())
			return 0;
	}

	internal_loadAlbumList(list);
	return 1; //success
}

function CreateFolder(parentAlbum,album)
{
	local title =album.getTitle();
	local summary = album.getSummary();
	local accessType = album.getAccessType();
	local parentId = album.getParentId; 
	local strAcessType = "private";
	if(token == "")
	{
		if(!doLogin())
			return 0;
	}
	
	if (accessType == 1)
		strAcessType = "public";
	
	nm.addQueryHeader("Expect","");
	nm.addQueryHeader("GData-Version", "2");
	nm.setUrl("http://picasaweb.google.com/data/feed/api/user/default");
	nm.addQueryHeader("Authorization","GoogleLogin auth="+token);
	
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
   if(token == "")
	{
		if(!doLogin())
			return 0;
	}
	nm.addQueryParam("GData-Version", "2");
	local albumID = "default";
	local albumStr = options.getFolderID();
	if(albumStr!="")
		albumID = albumStr;
	
	nm.setUrl("http://picasaweb.google.com/data/feed/api/user/default/albumid/"+albumID);
	nm.addQueryHeader("Authorization","GoogleLogin auth="+token);

	local ServerFileName = options.getServerFileName();
	if(ServerFileName=="") ServerFileName = ExtractFileName(FileName);
	local 	encodedFname = nm.urlEncode(ServerFileName);
	nm.addQueryHeader("Slug",encodedFname);
	nm.addQueryHeader("Expect","");
	nm.addQueryHeader("Content-Type",GetFileMimeType(FileName));
	nm.doUpload(FileName,"");

 	local data = nm.responseBody();
	
	local directUrl = regex_simple(data, "content type='image/.+src='(.+)'", 0);
		
	local thumbUrl = regex_simple(data, "<media:thumbnail.+<media:thumbnail.+<media:thumbnail url='(.+)'", 0);
	
	local viewUrl = 
		regex_simple(data, "<link rel=[\",']http://schemas.google.com/photos/2007#canonical[\",'] type=[\",']text/html[\",'] href='(.+)'", 0);

	options.setDirectUrl(directUrl);
	options.setThumbUrl(thumbUrl);
	options.setViewUrl(viewUrl);
	
	return 1;
}

function ModifyFolder(album)
{
	if(token == "")
	{
		if(!doLogin())
			return 0;
	}
	
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
	nm.setUrl("http://picasaweb.google.com/data/entry/api/user/default/albumid/"+id);
	 nm.addQueryHeader("Authorization","GoogleLogin auth="+token);
	
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