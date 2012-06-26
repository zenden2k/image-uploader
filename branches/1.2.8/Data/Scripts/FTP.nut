token <- "";

if(ServerParams.getParam("hostname") == "")
{
	ServerParams.setParam("hostname", "ftp.example.com") ;
}

if(ServerParams.getParam("folder") == "")
{
	ServerParams.setParam("folder", "/somefolder") ;
}

if(ServerParams.getParam("downloadPath") == "")
{
	ServerParams.setParam("downloadPath", "http://dl.example.com/somefolder");
}

function reg_replace(str, pattern, replace_with)
{
	local res = str.find(pattern);
		
	if(res != null){	
		return str.slice(0,res) +replace_with+ str.slice(res + pattern.len());
	}
	return str;
}

function  UploadFile(FileName, options)
{
	local newFilename = ExtractFileName(FileName);
	newFilename = random() +"_"+newFilename;
	local ansiFileName = Utf8ToAnsi(newFilename,0);
	local host = ServerParams.getParam("hostname");
	local folder = ServerParams.getParam("folder");
	
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	local downloadPath =  ServerParams.getParam("downloadPath");
	local authStr="";
	
	if(login.len())
	{
		authStr = login + ":" + pass + "@";
	}
	if(folder.slice(0,1) != "/")
		folder = "/" + folder;
		
	if(folder.slice(folder.len()-1) != "/")
		folder += "/";

	nm.setUrl("ftp://" + authStr+ host + folder+ansiFileName);
	nm.setMethod("PUT");
	nm.doUpload(FileName, "");
	if(nm.responseCode() != 226 && nm.responseCode() != 201)
	{
		local errorStr = "Code="+nm.responseCode()+"\r\n"+nm.errorString();
		
		if(nm.responseCode() == 530)
		errorStr += " (wrong username/password?)";
		print(errorStr);
		return 0;

	}
	if(downloadPath == ""){
		downloadPath = "http://"+host + folder;
		ServerParams.setParam("downloadPath",downloadPath);
	}
	if(downloadPath.slice(downloadPath.len()-1) != "/")
		downloadPath += "/";
	options.setDirectUrl(downloadPath+nm.urlEncode(newFilename));

	return 1;
}

function GetServerParamList()
{
	local a =
	{
		hostname = "Server ip or hostname [:port]"
		folder = "Remote folder"
		downloadPath = "Download path (ftp or http)"
	}
	return a;
}