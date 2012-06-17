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

function  UploadFile(FileName, options)
{	
	nm.setUrl("http://iceimg.com/upload.php");
	nm.addQueryParam("Filename",ExtractFileName(FileName));
	nm.addQueryParamFile("img",FileName, ExtractFileName(FileName),"");
	nm.addQueryParam("Upload", "Submit Query");
	nm.doUploadMultipartData();

 	local data = nm.responseBody();
	local directUrl = regex_simple(data, "full\":\"(.+)\"", 0);
	local thumbUrl = regex_simple(data, "thumb\":\"(.+)\"", 0);
	local viewUrl = regex_simple(data, "view\":\"(.+)\"", 0);
	directUrl = reg_replace(directUrl, "\\", "");
	thumbUrl = reg_replace(thumbUrl, "\\", "");
	viewUrl = reg_replace(viewUrl, "\\", "");

	options.setDirectUrl("http://iceimg.com/"+directUrl);
	options.setViewUrl("http://iceimg.com/"+viewUrl);
	options.setThumbUrl("http://iceimg.com/"+thumbUrl);	
	return 1;
}