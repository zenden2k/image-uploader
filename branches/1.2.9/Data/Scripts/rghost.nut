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
	nm.doGet("http://rghost.net/profile/sign_in");
	local serverData = nm.responseBody();
		
	local token = regex_simple(serverData,"\"(.+)\" name=\"csrf-token\"",0);	

	local email = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	if (email == "" ) {
		return 1;
	}
	
	nm.addQueryHeader("Expect","");
	nm.setUrl("http://rghost.net/profile/sign_in");
  
	nm.addQueryParam("utf8", "On"); 

	nm.addQueryParam("user[email]", email); 
	nm.addQueryParam("user[password]", pass); 
	nm.addQueryHeader("user[remember_me]", "1");
	nm.addQueryParam("authenticity_token",token);
   	nm.doPost("");
   	local data =  nm.responseBody();
	if ( regex_simple(data,"(Invalid email or password)",0) != "" ) {
		print("Authentication on Rghost.net failed with username '" + email + "'");
		return 0;
	}
  
	return 1; //Success login
} 


function  UploadFile(FileName, options)
{	
	 if ( !doLogin() )  {
		return 0;	
	}
	nm.doGet("http://rghost.net/multiple/upload_host");
	local serverData = nm.responseBody();
	
	local serverId = regex_simple(serverData,"upload_host\":\"(.+)\"",0);
	local token = regex_simple(serverData,"authenticity_token\":\"(.+)\"",0);

	nm.setUrl("http://"+serverId+"/files");
	nm.addQueryParam("authenticity_token",token);
	nm.addQueryParamFile("file",FileName, ExtractFileName(FileName),"");
	nm.doUploadMultipartData();

 	local data = nm.responseBody();
	local fileID = regex_simple(data,"http:\\/\\/rghost\\.\\w+\\/(\\d+)",0);
	local ex = regexp("\\[img\\](.+)\\[/img\\]");
	local res = ex.capture(data, 0);
	local directUrl = "";
	local thumbUrl = "";
	if(res != null){	
		thumbUrl = data.slice(res[1].begin, res[1].end);
		local res2 = ex.capture(data, res[1].begin);
		directUrl = data.slice(res2[1].begin, res2[1].end);
	}	

	options.setDirectUrl(directUrl);
	options.setViewUrl("http://rghost.ru/" + fileID);
	options.setThumbUrl(thumbUrl);	
	return 1;
}