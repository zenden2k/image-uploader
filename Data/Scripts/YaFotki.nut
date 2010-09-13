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

function reg_replace(str, pattern, replace_with)
{
	local res = str.find(pattern);
		
	if(res != null){	
		return str.slice(0,res) +replace_with+ str.slice(res + pattern.len());
	}
	return str;
}

function doLogin() 
{ 
 	
	login <- ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");

	if(login == "" || pass=="")
	{
		print("E-mail and password should not be empty!");
		return 0;
	}
	nm.addQueryHeader("Expect","");
	nm.setUrl("http://auth.mobile.yandex.ru/yamrsa/key/");
	nm.doGet("");
	local data = nm.responseBody();
	local reqid="";
	local publicKey="";
	
	reqid = regex_simple(data,"<request_id>(.+)</request_id>",0);
		
	publicKey = regex_simple(data,"<key>(.+)</key>",0);
		
	nm.setUrl("http://auth.mobile.yandex.ru/yamrsa/token/");
	nm.addQueryHeader("Expect","");
	nm.addQueryParam("request_id", reqid);

	local credentials = "<credentials login=\""+login+"\" password=\""+pass+"\"/>";
	local encrypted = YandexRsaEncrypter(publicKey, credentials);
	nm.addQueryParam("credentials", encrypted);

	nm.doPost("");
	data = nm.responseBody();
  
  
	token = regex_simple(data,"<token>(.+)</token>",0);
  
	if(token == "")
	{
		print("Authentication failed for username '"+login +"'");
		return false;
	}
	return 1; //Success login
} 

function internal_parseAlbumList(data,list,parentid)
{
	local start = 0;
	
	while(1)
	{
		local title="",id="",summary="";
		local ex = regexp("<entry");
		local res = ex.search(data, start);
		local link = "";
		local album = CFolderItem();
		if(res != null){	
			start = res.end;
		}
		else break;
		
		id = regex_simple(data,"album:(\\d+)</id>",start);
		title = regex_simple(data,"<title>(.+)</title>",start);
		summary = regex_simple(data,"<summary>(.+)</summary>",start);
		link = regex_simple(data,"link href=[\",'](.+)[\",'] rel=[\",']alternate[\",']",start);
		
		album.setId(id);
		album.setTitle(title);
		album.setSummary(summary);
		album.setViewUrl(link);
		album.setParentId(parentid);
		list.AddFolderItem(album);
		
	}
}

function internal_loadAlbumList(list)
{
	
	nm.addQueryHeader("Authorization", "FimpToken realm=\"fotki.yandex.ru\", token=\""+token+"\"");
	nm.addQueryHeader("Expect", "");
	nm.addQueryHeader("Connection", "close");
	nm.setUrl("http://api-fotki.yandex.ru/api/users/"+login+"/albums/");
	nm.doGet("");
	internal_parseAlbumList(nm.responseBody(), list,"");
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
	if(token == "")
	{
		doLogin();
	}

	nm.addQueryHeader("Authorization","FimpToken realm=\"fotki.yandex.ru\", token=\""+token+"\"");
  	nm.setUrl("http://api-fotki.yandex.ru/api/users/"+login+"/albums/");
	nm.addQueryHeader("Content-Type", "application/atom+xml; charset=utf-8; type=entry");
	nm.addQueryHeader("Connection", "close");
	local data = "<entry xmlns=\"http://www.w3.org/2005/Atom\" xmlns:f=\"yandex:fotki\">\r\n"+
	"<title>"+title+"</title>\r\n"+
	"<summary>"+summary+"</summary>\r\n"+
	"</entry>";
	
	nm.doPost(data);

	local response = nm.responseBody();
	
	local id = regex_simple(response,"album:(\\d+)</id>",0);
		
	album.setId(id);
	if(nm.responseCode() != 201) // HTTP Status code 201 = Created
		return 0;
	
	return 1;
}

function  UploadFile(FileName, options)
{
   if(token == "")
	{
		if(!doLogin())
			return 0;
	}

	nm.addQueryHeader("Authorization","FimpToken realm=\"fotki.yandex.ru\", token=\""+token+"\"");
  
	local url="";
	local albumStr = options.getFolderID();
	
	if(albumStr!="")
		url = "http://api-fotki.yandex.ru/api/users/"+login+"/album/"+albumStr+"/photos/";
	else
		url = "http://api-fotki.yandex.ru/api/users/"+login+"/photos/";
	
	nm.setUrl(url);
	
	local ServerFileName = options.getServerFileName();
	if(ServerFileName=="") 
	   ServerFileName = ExtractFileName(FileName);
	local 	encodedFname = nm.urlEncode(ServerFileName);
	nm.addQueryHeader("Slug",encodedFname);
	nm.addQueryHeader("Connection", "close");

	nm.addQueryHeader("Expect", "");
	
	nm.doUpload(FileName, "");
	
	if(nm.responseCode() != 201) 
	{
		print(nm.responseBody());
		return 0;
	}
	
	local data = nm.responseBody();
	local directUrl="";
	local thumbUrl ="";
	
	local tempUrl = regex_simple(data,"content src=\"(.+)\"",0);
	local viewUrl = regex_simple(data,"link href=\"(.+)\" rel=\"alternate\"",0);
	  
	directUrl = reg_replace(tempUrl,"_XL","_orig")+"."+GetFileExtension(FileName);
	thumbUrl = reg_replace(tempUrl,"_XL","_M")+"."+GetFileExtension(FileName);
	options.setDirectUrl(directUrl);
	options.setThumbUrl(thumbUrl);
	options.setViewUrl(viewUrl);
	
	return 1;
}

/*function ModifyFolder(album)
{
	
}*/

function GetFolderAccessTypeList()
{
	local a=["Приватный", "Для всех"];
	return a;
}