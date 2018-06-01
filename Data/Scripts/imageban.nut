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

	while( (res = resultStr.find(pattern,start)) != null ) {	

		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
		start = res + replace_with.len();
	}
	return resultStr;
}



function getThumbnailWidth() {
	local result = "180";
	try{
		result = options.getParam("THUMBWIDTH");
	}
	catch(ex)
	{
		//print("This plugin needs Image Uploader version >= 1.2.7.\n(ifolder.ru)");
	}
	return result;
}

function anonymousUpload(FileName, options) {
	nm.setUrl("https://imageban.ru/up");
	nm.addQueryHeader("User-Agent", "Shockwave Flash");
	nm.addQueryParam("Filename", ExtractFileName(FileName));
	nm.addQueryParam("albmenu", "0");
	nm.addQueryParam("grad", "0");
	nm.addQueryParam("rsize", "0");
	nm.addQueryParam("inf", "1");
	nm.addQueryParam("prew", getThumbnailWidth());
	nm.addQueryParam("ptext", "");
	nm.addQueryParam("rand", format("%d",random()%22222));
	nm.addQueryParam("ttl", "0");
	nm.addQueryParamFile("Filedata",FileName, ExtractFileName(FileName),"");
	nm.addQueryParam("Upload", "Submit Query");
	
	nm.doUploadMultipartData();

 	local data = nm.responseBody();

	local directUrl = regex_simple(data, "link\":\"(.+)\"", 0);
	local thumbUrl = regex_simple(data, "thumbs\":\"(.+)\"", 0);
	local viewUrl = regex_simple(data, "view\":\"(.+)\"", 0);
	directUrl = reg_replace(directUrl, "\\", "");
	thumbUrl = reg_replace(thumbUrl, "\\", "");
	viewUrl = reg_replace(viewUrl, "\\", "");

	options.setDirectUrl(directUrl);
	options.setViewUrl(viewUrl);
	options.setThumbUrl(thumbUrl);	
	if ( directUrl != "") {
		return 1;
	}
	return 0;
}

function  UploadFile(FileName, options)
{	
	local login = ServerParams.getParam("Login");
	local password =  ServerParams.getParam("Password");
	if ( login == "" || password == "" ) {
		return anonymousUpload(FileName, options);
	}
	
	nm.setUrl("http://api.imageban.ru/upload_api.php");
	nm.addQueryParamFile("userfile",FileName, ExtractFileName(FileName),"");
	nm.addQueryParam("user", login);
	nm.addQueryParam("passwd",password);
	nm.doUploadMultipartData();
	local data = nm.responseBody();
	
	local url2 = regex_simple(data, "<url2>(.+)</url2>", 0);
	local url3 = regex_simple(data, "<url3>(.+)</url3>", 0);
	local viewUrl  	= regex_simple(data, "<url1>(.+)</url1>", 0);
	local thumbUrl  	= regex_simple(url2, "\\[IMG\\](.+)\\[/IMG\\]",0);
	local directUrl  	= regex_simple(url3, "\\[IMG\\](.+)\\[/IMG\\]",0);
	
	options.setDirectUrl(directUrl);
	options.setViewUrl(viewUrl);
	options.setThumbUrl(thumbUrl);	
	
	return 1;
}