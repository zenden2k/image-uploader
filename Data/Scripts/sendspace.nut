apiKey <- "KAAK63MDZJ";

function IsAuthenticated() {
    if (Sync.getValue("sessionKey") != "") {
        return 1;
    }
    return 0;
}

function Authenticate() { 
    if (Sync.getValue("sessionKey") != "") {
        return 1;
    }
    local login = ServerParams.getParam("Login");
    local pass =  ServerParams.getParam("Password");

    if(login == "" || pass=="") {
        WriteLog("error", "E-mail and password should not be empty!");
        return 0;
    }
    
    nm.addQueryHeader("Expect","");
    
    nm.setUrl("https://api.sendspace.com/rest/?method=auth.createtoken&api_key=" + apiKey + "&api_version=1.0&response_format=xml");
    nm.doGet("");
    local data =  nm.responseBody();
    
    local ex = regexp("<token>(.+)</token>");
    local token = "";
    
    local res = ex.capture(data);
    if(res != null){	
        token = data.slice(res[1].begin,res[1].end);
    }
    
    if (token == "") {
        WriteLog("error", "Unable to obtain auth token!");
        return 0;
    }  
 
    local tokened_password = md5(token + md5(pass));
    local authUrl = "https://api.sendspace.com/rest/?method=auth.login&token="+ token+"&user_name="+login+"&tokened_password="+ tokened_password;
    nm.doGet(authUrl);

    data = nm.responseBody();
    
    ex = regexp("<session_key>(.+)</session_key>");

    res = ex.capture(data);
    local sessionKey = "";
    if(res != null){	
        sessionKey = data.slice(res[1].begin,res[1].end);
    }
        
    if(sessionKey == "") {
        print("Error while authentication with username " + login);
        return 0;
    }
    Sync.setValue("sessionKey", sessionKey);
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
        else {
            break;
        }
                
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

function GetFolderList(list) {
    local sessionKey = Sync.getValue("sessionKey");
    nm.setUrl("https://api.sendspace.com/rest/?method=folders.getall&session_key=" + sessionKey);
    nm.addQueryHeader("Expect","");
    nm.doGet("");
    internal_parseAlbumList(nm.responseBody(), list);
    return 1; //success
}

function CreateFolder(parentAlbum, album) {
    local title =album.getTitle();
    local summary = album.getSummary();
    local accessType = album.getAccessType();
    local parentId = album.getParentId(); 
    local strAcessType = "private";
    local sessionKey = Sync.getValue("sessionKey");

    if (accessType == ""){
        accessType = 0;
    }
    if (parentId == "") {
        parentId = 0;
    }
    
    local url = "https://api.sendspace.com/rest/?method=folders.create&session_key=" + sessionKey + "&name="+nm.urlEncode(title)+"&shared="+accessType+"&parent_folder_id="+parentId;
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

function UploadFile(FileName, options) {
    local sessionKey = Sync.getValue("sessionKey");
  
    local albumID = options.getFolderID();
    if(albumID == "") {
        albumID = "0";
    }
    
    local anonymous = (ServerParams.getParam("Login") == "");
    local func = "upload.getinfo";
    if (anonymous) {
        func = "anonymous.uploadgetinfo";
    }
    
    if(anonymous) {
        nm.setUrl("https://api.sendspace.com/rest/?method=anonymous.uploadgetinfo&api_key=KAAK63MDZJ&api_version=1.0");
    }  else {
        nm.setUrl("https://api.sendspace.com/rest/?method=" + func + "&session_key=" + sessionKey);
    }
    
    nm.doGet("");
    local data = nm.responseBody();

    local uploadUrl ="";

    local ex = regexp("upload url=\"(.+)\"");

    local res = ex.capture(data);

    if(res != null){	
        uploadUrl= data.slice(res[1].begin,res[1].end);     
    } else {
        return 0;
    }
    
    local max_file_size = "";

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
    if(anonymous) {
        ex = regexp("<download_url>(.+)</download_url>");
        res = ex.capture(data);
        if(res != null){	
            download_page_url= data.slice(res[1].begin,res[1].end);
        }
    } else {
        ex	= regexp("id=(\\w+)");
        local res = ex.capture(data);
        
        if(res != null){	
            fileId = data.slice(res[1].begin,res[1].end);  
        }        
        
        nm.doGet("https://api.sendspace.com/rest/?method=files.getInfo&session_key=" + sessionKey+"&file_id="+fileId);
    
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
            direct_download_url = data.slice(res[1].begin,res[1].end);
        }
    }	
        
    if(direct_download_url != "") {
        options.setDirectUrl(direct_download_url);
    }
        
    options.setViewUrl(download_page_url);
    return 1;
}

function ModifyFolder(album) {
    local sessionKey = Sync.getValue("sessionKey");

    if (sessionKey == ""  && ServerParams.getParam("Login") != "") {
        if(!DoLogin()) {
            return 0;
        }
    }
    
    sessionKey = Sync.getValue("sessionKey");

    local title = album.getTitle();
    local id = album.getId();
    if(id == "") { 
        id = "0";
    }
    local summary = album.getSummary();
    local accessType = album.getAccessType();
    local parentId = album.getParentId; 
    if (accessType == "") {
        accessType = 0;
    }

    local url = "https://api.sendspace.com/rest/?method=folders.setInfo&session_key=" + sessionKey + "&folder_id="+id+"&name="+nm.urlEncode(title)+"&shared="+accessType;
    nm.doGet(url);
}

function GetFolderAccessTypeList() {
    return ["Приватный", "Для всех"];
}