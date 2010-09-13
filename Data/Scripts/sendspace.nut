session_key <- "";

function doLogin() 
{ 
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");

	if(login == "" || pass=="")
	{
		print("E-mail and password should not be empty!");
		return 0;
	}
	

	nm.addQueryHeader("Expect","");
	
nm.setUrl("http://api.sendspace.com/rest/?method=auth.createtoken&api_key=KAAK63MDZJ&api_version=1.0&response_format=xml");

   nm.doGet("");
   local data =  nm.responseBody();
  

  
  local ex = regexp("<token>(.+)</token>");
  local token = "";
		local res = ex.capture(data);
		if(res != null){	
			token = data.slice(res[1].begin,res[1].end);
		
		}
		
 
	local tokened_password = md5(token + md5(pass));
	local authUrl = "http://api.sendspace.com/rest/?method=auth.login&token="+ token+"&user_name="+login+"&tokened_password="+ tokened_password;
	nm.doGet(authUrl);

	data =  nm.responseBody();
	
	ex = regexp("<session_key>(.+)</session_key>");

	res = ex.capture(data);
		if(res != null){	
			session_key = data.slice(res[1].begin,res[1].end);
		
		}
		
	//print("token = "+token+"\r\n"+session_key);
	if(session_key == "")
	{
		print("Eror while authencation using username "+login);
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
		local ex = regexp("<folder");
		local res = ex.search(data, start);
		local link = "";
		local access = "private";
		local parentId = "";
		local album = CFolderItem();
		local shared = "0";
		if(res != null){	
			start = res.end;
		}
		else break;
		
		
		ex = regexp("id=\"(.+)\"");
		res = ex.capture(data, start);
		if(res != null){	
			id = data.slice(res[1].begin,res[1].end);
			
		}
		
		ex = regexp("name=\"(.+)\"");
		res = ex.capture(data, start);
		if(res != null){	
			title = data.slice(res[1].begin,res[1].end);
		
		}
		
		ex = regexp("shared=\"(.+)\"");
		res = ex.capture(data, start);
		if(res != null){	
			shared = data.slice(res[1].begin,res[1].end);
		
		}
		
		
		ex = regexp("parent_folder_id=\"(.+)\"");
		res = ex.capture(data, start);
		if(res != null){	
			parentId = data.slice(res[1].begin,res[1].end);
			if(parentId == "0" && id =="0") parentId="";
		}
		
		/*ex = regexp("<gphoto:access>(.+)</gphoto:access>");
		res = ex.capture(data, start);
		if(res != null){	
			 access = data.slice(res[1].begin,res[1].end);
		
		}*/
		
		if( access == "private")
			album.setAccessType(0);
		else
			album.setAccessType(1);
		
		ex = regexp("public_url=\"(.+)\"");
		//ex = regexp("href=\"(.+)\"");
		res = ex.capture(data, start);
		if(res != null){	
			link = data.slice(res[1].begin,res[1].end);
		
		}

		album.setId(id);
		album.setTitle(title);
		if(shared == "1")
		album.setAccessType(1);
		else album.setAccessType(0);
		album.setSummary(summary);
		album.setViewUrl(link);
		album.setParentId(parentId);
		list.AddFolderItem(album);
		
	}
}

function internal_loadAlbumList(list)
{
	nm.setUrl("http://api.sendspace.com/rest/?method=folders.getall&session_key="+session_key);
  	nm.addQueryHeader("Expect","");
	nm.doGet("");
	internal_parseAlbumList(nm.responseBody(),list);
}

function GetFolderList(list)
{
	if(session_key == "")
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
	local parentId = album.getParentId(); 
	local strAcessType = "private";
	if(session_key == "")
	{
		if(!doLogin())
			return 0;
	}
	//print("We create album name = "+title);
	
	if (accessType == "")
		accessType = 0;
		if(parentId == "")
		parentId= 0;
	
	//nm.addQueryHeader("Expect","");
	

	local url = "http://api.sendspace.com/rest/?method=folders.create&session_key="+session_key+"&name="+nm.urlEncode(title)+"&shared="+accessType+"&parent_folder_id="+parentId;
	//print(url);
	 nm.doGet(url);
	 local data = nm.responseBody();
	 
	 local id="", link="";
	 local ex = regexp("id=\"(.+)\"");
		local res = ex.capture(data);
		if(res != null){	
			id = data.slice(res[1].begin,res[1].end);
			
		}
		
	ex = regexp("public_url=\"(.+)\"");
		res = ex.capture(data);
		if(res != null){	
			link = data.slice(res[1].begin,res[1].end);
		
		}	
		
		album.setId(id);
	
		album.setViewUrl(link);

		return 1;
}

function  UploadFile(FileName, options)
{
   if(session_key == "" && ServerParams.getParam("Login") != "")
	{
		if(!doLogin())
			return 0;
	}
  
	local albumID = options.getFolderID();
	if(albumID == "") albumID = "0";
	
	local anonymous = (ServerParams.getParam("Login") == "");
	local func = "upload.getinfo";
	if (anonymous) 
		func = "anonymous.uploadgetinfo";
	
	if(anonymous)
	nm.setUrl("http://api.sendspace.com/rest/?method=anonymous.uploadgetinfo&api_key=KAAK63MDZJ&api_version=1.0");
	
	
	else
	nm.setUrl("http://api.sendspace.com/rest/?method=" + func + "&session_key=" + session_key);
	
	nm.doGet("");

	local 	data = nm.responseBody();


	local uploadUrl ="";
//print(data + "_"+uploadUrl);
	 local ex = regexp("upload url=\"(.+)\"");

		local res = ex.capture(data);
print("okffff");
		if(res != null){	
			uploadUrl= data.slice(res[1].begin,res[1].end);
			
		}
	else return 0;
	
	print(data);
	local max_file_size="";

	ex = regexp("max_file_size=\"(.+)\"");
		res = ex.capture(data);
		if(res != null){	
			max_file_size= data.slice(res[1].begin,res[1].end);
			
		}
		
		local upload_identifier="";

	ex = regexp("upload_identifier=\"(.+)\"");
		res = ex.capture(data);
		if(res != null){	
			upload_identifier= data.slice(res[1].begin,res[1].end);
			
		}
		
		
		local extra_info="";

	ex = regexp("extra_info=\"(.+)\"");
		res = ex.capture(data);
		if(res != null){	
			extra_info= data.slice(res[1].begin,res[1].end);
			
		}
		
	nm.setUrl(uploadUrl);
	nm.addQueryParam("MAX_FILE_SIZE", max_file_size);
	nm.addQueryParam("UPLOAD_IDENTIFIER", upload_identifier);
	
	nm.addQueryParam("extra_info", extra_info);
	nm.addQueryParam("MAX_FILE_SIZE", max_file_size);
	nm.addQueryParam("folder_id", albumID);
	nm.addQueryParamFile("userfile",FileName, ExtractFileName(FileName),"");

	nm.doUploadMultipartData();
 	data = nm.responseBody();
	local directUrl="";
	local thumbUrl ="";
	
	local fileId="";
	local ex;
	local download_page_url="";
	local direct_download_url="";
	if(anonymous)
	{
		ex = regexp("<download_url>(.+)</download_url>");
		res = ex.capture(data);
		if(res != null){	
			download_page_url= data.slice(res[1].begin,res[1].end);
			
		}

	}
	
	else
	{
		ex	= regexp("id=(\\w+)");
		local res = ex.capture(data);
		
		if(res != null){	
			fileId = data.slice(res[1].begin,res[1].end);
			
		}
		//print(data);
		//print("fileId="+fileId);
		
		
		nm.doGet("http://api.sendspace.com/rest/?method=files.getInfo&session_key=" + session_key+"&file_id="+fileId);
	
		data = nm.responseBody();
		
		
		//[\\w/:&?%]
		ex = regexp("download_page_url=\"(.+)\"");
		res = ex.capture(data);
		if(res != null){	
			download_page_url= data.slice(res[1].begin,res[1].end);
			
		}
		
		

		ex = regexp("direct_download_url=\"([\\w/:?&]*)\"");
		res = ex.capture(data);
		if(res != null){	
			direct_download_url= data.slice(res[1].begin,res[1].end);
			
		}
	}	
		
		
	if(direct_download_url != "")
		options.setDirectUrl(direct_download_url);
		
	options.setViewUrl(download_page_url);
		
		
		
	return 1;
}

function ModifyFolder(album)
{
	if(session_key == "")
	{
		if(!doLogin())
			return 0;
	}
	
	local title = album.getTitle();
	local id = album.getId();
	if(id == "") 
	  id = "0";
	local summary = album.getSummary();
	local accessType = album.getAccessType();
	local parentId = album.getParentId; 
	if (accessType == "")
		accessType = 0;


	local url = "http://api.sendspace.com/rest/?method=folders.setInfo&session_key="+session_key+"&folder_id="+id+"&name="+nm.urlEncode(title)+"&shared="+accessType;
	nm.doGet(url);

}

function GetFolderAccessTypeList()
{
	local a=["Приватный", "Для всех"];
	return a;
}