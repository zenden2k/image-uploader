const CURLOPT_USERNAME = 10173;
const CURLOPT_PASSWORD = 10174;

token <- "";

if(ServerParams.getParam("hostname") == "") {
	ServerParams.setParam("hostname", "ftp.example.com") ;
}

if(ServerParams.getParam("folder") == "") {
	ServerParams.setParam("folder", "/somefolder") ;
}

if(ServerParams.getParam("downloadPath") == "") {
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

function TestConnection() {
    local host = ServerParams.getParam("hostname");
    local login = ServerParams.getParam("Login");
    local pass =  ServerParams.getParam("Password");
    local folder = ServerParams.getParam("folder");

        
	if(login.len()) {
        nm.setCurlOption(CURLOPT_USERNAME, login);
        nm.setCurlOption(CURLOPT_PASSWORD, pass);
    }

    if(folder.slice(0,1) != "/") {
        folder = "/" + folder;
    }
        
    if(folder.slice(folder.len()-1) != "/") {
        folder += "/";
    }

	local url = "ftp://" + host + folder;

    if (nm.doGet(url)){
        if (nm.responseBody() != "") {
            return {
                status = 1,
                message = ""
            };
        }
    }

    return {
        status = 0,
        message = ""
    };
}

function  UploadFile(FileName, options) {
	local newFilename = ExtractFileName(FileName);
	newFilename = random() +"_"+newFilename;
	local ansiFileName = newFilename;
	local host = ServerParams.getParam("hostname");
	local folder = ServerParams.getParam("folder");
	
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	local downloadPath = ServerParams.getParam("downloadPath");
	
	if(login.len()) {
        nm.setCurlOption(CURLOPT_USERNAME, login);
        nm.setCurlOption(CURLOPT_PASSWORD, pass);
    }

	if(folder.slice(0,1) != "/")
		folder = "/" + folder;
		
	if(folder.slice(folder.len()-1) != "/")
		folder += "/";
	ansiFileName = reg_replace(ansiFileName, " ", "_");
	local url = "ftp://" + host + folder+ ansiFileName;

	nm.setUrl(url);
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
	options.setDirectUrl(downloadPath+reg_replace(nm.urlEncode(newFilename),"%2E","."));

	return 1;
}

function GetServerParamList() {
	return {
		hostname = "Server ip or hostname [:port]"
		folder = "Remote folder"
		downloadPath = "Download path (ftp or http)"
	}
}