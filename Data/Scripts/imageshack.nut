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
	local resultStr = str;	
	local res;
	while(res = resultStr.find(pattern)){	
		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
	}
	return resultStr;
}

function auth()
{
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	if(login!="" && pass != "")
	{
		nm.setUrl("https://api.imageshack.com/v2/user/login");
		nm.addQueryParam("user", login);
		nm.addQueryParam("password", pass);
		nm.addQueryParam("api_key", "BXT1Z35V8f6ee0522939d8d7852dbe67b1eb9595");
		nm.doPost("");
		
		token = regex_simple(nm.responseBody(), "auth_token\":\"(.+)\"", 0);
		
	}
	return true;
}

function  UploadFile(FileName, options)
{
	if(token == "") 
	{
		if(!auth()) return false;
	}
	nm.setUrl("https://api.imageshack.com/v2/images");
	nm.addQueryParam("auth_token", token);
	nm.addQueryParam("api_key", "BXT1Z35V8f6ee0522939d8d7852dbe67b1eb9595");
	nm.addQueryParamFile("file",FileName, ExtractFileName(FileName),GetFileMimeType(FileName));
	local thumbwidth = 180;
	try //for compatibility with IU versions < 1.2.7
	{
	  thumbwidth = options.getParam("THUMBWIDTH");
	}
	catch(ex){}

	local data = "";
	nm.doUploadMultipartData();
	data = nm.responseBody();
	if(data == "")
	{
		print ("Empty response");
		return 0;
	}
	local directUrl = regex_simple(data, "direct_link\":\"(.+)\"", 0);
	
	if ( directUrl != "" ) {
		directUrl = "http://" + reg_replace(directUrl, "\\", "");
	}
	
	options.setDirectUrl(directUrl);
	//options.setViewUrl(viewUrl);
	//options.setThumbUrl(thumbUrl);	
	return 1;
}