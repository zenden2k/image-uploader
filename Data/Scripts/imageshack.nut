include("Utils/RegExp.nut");
include("Utils/String.nut");

token <- "";

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
		directUrl = "http://" + strReplace(directUrl, "\\", "");
	}
	
	options.setDirectUrl(directUrl);
	//options.setViewUrl(viewUrl);
	//options.setThumbUrl(thumbUrl);	
	return 1;
}