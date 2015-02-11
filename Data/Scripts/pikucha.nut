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
	local start = 0;
	while(res = resultStr.find(pattern,start)){	
		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
		start = res + replace_with.len();
	}
	return resultStr;
}


function auth()
{
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	if(login!="" && pass != "")
	{
		nm.setUrl("http://pikucha.ru/api?object=user&action=authenticate");
		nm.addQueryParam("email", login);
		nm.addQueryParam("password", md5(pass));
		nm.doPost("");
		local id = regex_simple(nm.responseBody(), "id\":\"(.+)\"", 0);
		if(id == ""){ print("Ошибка аутентификации!"); return false;}
		
		nm.setUrl("http://pikucha.ru/api?object=user&action=data");
		nm.doPost("");
		token = regex_simple(nm.responseBody(), "token\":\"(.+)\"", 0);
		
	}
	else
	{
		nm.setUrl("http://pikucha.ru/api?object=user&action=create");
		nm.doPost("");
		token = regex_simple(nm.responseBody(), "token\":\"(.+)\"", 0);
	}
	return true;
}

function  UploadFile(FileName, options)
{
	if(token == "") 
	{
		if(!auth()) return false;
	}
	nm.setUrl("http://u.pikucha.ru/api?object=image&action=upload");
	nm.addQueryHeader("Referer","http://pikucha.ru/");
	nm.addQueryHeader("Connection", "Keep-Alive, TE");
	nm.addQueryHeader("TE", "deflate, gzip");
	nm.addQueryParam("token", token);
	nm.addQueryParam("name", ExtractFileName(FileName));
	nm.addQueryParam("Filename", ExtractFileName(FileName));
	nm.addQueryParamFile("file",FileName, ExtractFileName(FileName),GetFileMimeType(FileName));
	local thumbwidth = 180;
	try //for compatibility with IU versions < 1.2.7
	{
	  thumbwidth = options.getParam("THUMBWIDTH");
	}
	catch(ex){}
	//print("WIDTH="+thumbwidth);
	nm.addQueryParam("thumbnail_dimension", "180");
	nm.addQueryParam("thumbnail_size", "0");
	local data = "";
	nm.doUploadMultipartData();
	data = nm.responseBody();
	if(data == "")
	{
		print ("Empty response");
		return 0;
	}
	local directUrl = regex_simple(data, "url_direct\":\"(.+)\"", 0);
	local thumbUrl = regex_simple(data, "url_thumbnail\":\"(.+)\"", 0);
	local viewUrl = regex_simple(data, "url\":\"(.+)\"", 0);
	directUrl = reg_replace(directUrl, "\\", "");
	thumbUrl = reg_replace(thumbUrl, "\\", "");
	viewUrl = reg_replace(viewUrl, "\\", "");
	//options.setDirectUrl(directUrl);
	options.setViewUrl(viewUrl);
	options.setThumbUrl(thumbUrl);	
	return 1;
}