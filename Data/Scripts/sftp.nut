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
	local resultStr = str;	
	local res;
	local start = 0;

	while( (res = resultStr.find(pattern,start)) != null ) {	

		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
		start = res + replace_with.len();
	}
	return resultStr;
}


function  UploadFile(FileName, options)
{	
	local newFilename = ExtractFileName(FileName);
	newFilename = random() +"_"+newFilename;
	local ansiFileName = newFilename;
	local host = ServerParams.getParam("hostname");
	local folder = ServerParams.getParam("folder");
	
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	local downloadPath =  ServerParams.getParam("downloadPath");
	local authStr="";
		
	if(login.len())
	{
		if (pass.len()) {
			authStr = login + ":" + pass + "@";
		} else {
			authStr = login + "@";
		}
		
	}
	if(folder.slice(0,1) != "/")
		folder = "/" + folder;
		
	if(folder.slice(folder.len()-1) != "/")
		folder += "/";
	local url = "sftp://" + authStr + host + folder + nm.urlEncode(ansiFileName);
	local privateKeyPath = ServerParams.getParam("privateKeyPath");
	if (privateKeyPath.len()) {
		nm.setCurlOption(10153, privateKeyPath); //CURLOPT_SSH_PRIVATE_KEYFILE
	}
	
	nm.enableResponseCodeChecking(false);
	nm.setUrl(url);
	nm.setMethod("PUT");
	local result = nm.doUpload(FileName, "");
	
	if(!result) {
		local errorStr = "Code=" + nm.responseCode()+"\r\n"+nm.errorString();
		
		if(nm.responseCode() == 530) {
			errorStr += " (wrong username/password?)";
		}
		WriteLog("error", errorStr);
		return 0;
	}
	if(downloadPath == ""){
		downloadPath = "http://"+host + folder;
		ServerParams.setParam("downloadPath",downloadPath);
	}
	if(downloadPath.slice(downloadPath.len()-1) != "/")
		downloadPath += "/";
	options.setDirectUrl(downloadPath+reg_replace(nm.urlEncode(newFilename),"%2E","."));

	return 1;
}

function GetServerParamList()
{
	local a =
	{
		hostname = "Server ip or hostname [:port]"
		folder = "Remote folder"
		downloadPath = "Download path (ftp or http)",
		privateKeyPath = "Private key path"
	}
	return a;
}