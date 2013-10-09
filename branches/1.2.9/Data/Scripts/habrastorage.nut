if(ServerParams.getParam("userkey") == "")
{
	ServerParams.setParam("userkey", "<type your key here>") ;
}

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

function authenticate()
{
	if(ServerParams.getParam("userkey") != "" && ServerParams.getParam("userkey") != "<type your key here>") return 1;
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	if(login == "" || pass == "")
	{
		print("Login and password should not be empty!");
		return 0;
	}
	local captcha = "";
	local ext = 1;
	do
	{
		nm.setUrl("http://habrahabr.ru/ajax/auth/");
		nm.addQueryHeader("X-Requested-With","XMLHttpRequest");
		nm.addQueryHeader("Referer","http://habrahabr.ru/login/");
		nm.addQueryHeader("Content-type","application/x-www-form-urlencoded; charset=utf-8");
		nm.addQueryParam("login", login);
		nm.addQueryParam("password", pass);
		nm.addQueryParam("act", "login");
		nm.addQueryParam("redirect_url", "http://habrahabr.ru");
		if(captcha != "")
			nm.addQueryParam("captcha", captcha);
		nm.doPost("");
		//nm.doPost("act=login&redirect_url=http%3A%2F%2Fhabrahabr.ru%2F&login="+login+"&password="+pass+"&captcha=&=true");
		local data = nm.responseBody();
		if(regex_simple(data, "(habrahabrLoginCaptcha)",0) != "" || regex_simple(data, "(refresh_captcha)",0)!="")
		{
			local captchaUrl = "http://habrahabr.ru/core/captcha/?"+random();
			try
			{
				captcha = AskUserCaptcha(nm, captchaUrl);
			}
			catch(ex){
				print("Couldn't show captcha dialog!\nYou are using an old version of Image Uploader!\n"); return 0;
			}
			//print("Captcha: "+captcha);
			if(captcha == "")
			{
				return 0;
			}
			ext = 0;
		}
		else ext = 1;
	}
	while(ext != 1)
	
	nm.addQueryHeader("Referer","http://habrastorage.org/");
	nm.setUrl("http://habrahabr.ru/whoami/");
	nm.doGet("");
	local userKey = regex_simple(nm.responseBody(),"userKey=\"(.+)\"",0 );
	if(userKey != "")
	{
		ServerParams.setParam("userkey", userKey);
		return 1;
	}
	return 0;
}
function  UploadFile(FileName, options)
{	
	if(!authenticate()) return 0;
	local login = ServerParams.getParam("Login");
	local userKey = ServerParams.getParam("userkey");
	nm.setUrl("http://habrastorage.org/uploadController/?username="+login+"&userkey="+userKey);
	nm.addQueryParamFile("Filedata",FileName, ExtractFileName(FileName),"");
	nm.addQueryHeader("Referer","http://habrastorage.org/");
	nm.addQueryParam("Filename",ExtractFileName(FileName)); 
	nm.addQueryParam("Upload", "Submit Query");
	nm.doUploadMultipartData();
	local data = nm.responseBody();
	local directUrl = regex_simple(data, "url\":\"(.+)\"", 0);
	directUrl =  reg_replace(directUrl, "\\", "")
	local thumbUrl = regex_simple(data, "resize\":\"(.+)\"", 0);
	thumbUrl =  reg_replace(thumbUrl, "\\", "")
	options.setDirectUrl(directUrl);
	options.setThumbUrl(thumbUrl);
	return 1;
}


function GetServerParamList()
{
	local a =
	{
		userkey = "UserKey"
	}
	return a;
}