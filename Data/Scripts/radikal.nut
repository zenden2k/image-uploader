authorized <- false;
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

function _WriteLog(type,message) {
	try {
		WriteLog(type, message);
	} catch (ex ) {
		print(type + " : " + message);
	}
}

function tr(key, text) {
	try {
		return Translate(key, text);
	}
	catch(ex) {
		return text;
	}
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



function DoLogin()
{
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	if(login!="" && pass != "")
	{
		nm.setUrl("http://radikal.ru/Auth/Login");
		nm.addQueryParam("Login", login);
		nm.addQueryParam("Password", pass);
		nm.addQueryParam("IsRemember", "true");
		nm.addQueryParam("ReturnUrl", "/");
		nm.doPost("");
		try {
			local t = ParseJSON(nm.responseBody());
			if ( t == null || t.IsError )  {
				_WriteLog("error", tr("radikal.login_error", "Authentication error on radikal.ru"));
				return 0;
			} 
			authorized = true;
		} catch ( ex ) { }
	}
	return 1;
}

function  UploadFile(FileName, options)
{
	if(!DoLogin()) return 0;
	local thumbwidth = 180;
	try //for compatibility with IU versions < 1.2.7
	{
	  thumbwidth = options.getParam("THUMBWIDTH");
	}
	catch(ex){}
	
	nm.setUrl("http://radikal.ru/api3/rest/uplimg/prepared");
	nm.addQueryHeader("Referer","http://radikal.ru/Content/Clients/FotoFlashApplet3.swf?v=13");
	nm.addQueryHeader("User-Agent","Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/36.0.1985.125 Safari/537.36");
	
	nm.addQueryParam("RESPTYPE", "xml");
	nm.addQueryParam("FROMOBJECT", "22");
	nm.addQueryParam("MAXSIZEIMG", "1024");
	nm.addQueryParam("MAXSIZEIMGPREV", thumbwidth);
	nm.addQueryParam("ISINSCRIMGPREV", "true");
	nm.addQueryParam("INSCRIMGPREV", "Увеличить");
	nm.addQueryParam("IDALBUM", "noall");
	nm.addQueryParam("COMMENTIMG", "");
	nm.addQueryParam("URLFORIMG", "");
	nm.addQueryParam("TAGS", "");
	nm.addQueryParam("ISPUBLIC", "false");
		
	nm.addQueryParam("Filename", ExtractFileName(FileName));
	nm.addQueryParamFile("Filedata",FileName, ExtractFileName(FileName),GetFileMimeType(FileName));

		nm.addQueryParam("Upload", "Submit Query");
	local data = "";
	nm.doUploadMultipartData();
	data = nm.responseBody();
	if(data == "")
	{
		print ("Empty response");
		return 0;
	}
	local directUrl = regex_simple(data, "<ImgUrl>(.+)<", 0);
	local thumbUrl = regex_simple(data, "<ImgPreviewUrl>(.+)<", 0);
	//local viewUrl = regex_simple(data, "url\":\"(.+)\"", 0);

	options.setDirectUrl(directUrl);
	//options.setViewUrl(viewUrl);
	options.setThumbUrl(thumbUrl);	
	return 1;
}