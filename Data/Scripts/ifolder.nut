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



function  UploadFile(FileName, options)
{ 
	try{
		options.getParam("THUMBWIDTH");
		}
		catch(ex)
		{
			print("This plugin needs Image Uploader version >= 1.2.7.\n(ifolder.ru)");
			return 0;
		}
	nm.doGet("http://ifolder.ru/");
	local data = nm.responseBody();
	local uploadparams = regex_simple(data, "upload_params\"\\s+value=\"(.+)\"", 0);
	local clone1 = regex_simple(data, "clone\"\\s+value=\"(.+)\"", 0);
	//local clone = "";//regex_simple(data, "clone\"\\s+value=\"(.+)\"", 0);
	local progressbar = regex_simple(data, "progress_bar\"\\s+value=\"(.+)\"", 0);
	local uploadhost = regex_simple(data, "upload_host\"\\s+value=\"(.+)\"", 0);
	local MAXFILESIZE = regex_simple(data, "MAX_FILE_SIZE\"\\s+value=\"(.+)\"", 0);
	
	local url = "http://upl." +uploadhost + "/?serial=" + progressbar;

	nm.setUrl(url);
		nm.addQueryParamFile("filename",FileName, ExtractFileName(FileName),GetFileMimeType(FileName));

	nm.addQueryParam("upload_params", uploadparams);
	nm.addQueryParam("progress_bar", progressbar);
	nm.addQueryParam("clone", clone1);
	nm.addQueryParam("upload_host", uploadhost);
	nm.addQueryParam("MAX_FILE_SIZE", MAXFILESIZE);

	nm.doUploadMultipartData();
	while(true)
	{
		data = nm.responseBody();
		local session = regex_simple(data, "src=\"/random/images/\\?session=(.+)\"", 0);
		
		local captcha =  regex_simple(data, "src=\"(/random/images/\\?session.+)\"", 0); 
		if(captcha == "") break;
		local number ="";
		if(captcha!="")
		{
			number = AskUserCaptcha(nm, "http://ifolder.ru"+captcha);
			if(number == "") return 0;
		}
		local confirmUrl = "http://ifolder.ru/upload/?session=" + session;
		nm.addQueryParam("confirmed_number", number);
		nm.setUrl(confirmUrl);
		nm.addQueryParam("action", "Подтвердить");
		nm.doPost("");
		data = nm.responseBody();
		
	}
	local fileId =  regex_simple(data, "/control/\\?file_id=([0-9]+)", 0); 
	if(fileId != "")
		options.setViewUrl("http://ifolder.ru/" + fileId);


	return 1;
}