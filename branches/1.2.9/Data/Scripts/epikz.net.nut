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

function  UploadFile(FileName, options)
{	
	nm.setUrl("http://epikz.net/upload.php");
	nm.addQueryParamFile("image[]",FileName, ExtractFileName(FileName),"");
	nm.doUploadMultipartData();
 	local data = nm.responseBody();
	local directUrl = regex_simple(data,"value=\"http:\\/\\/epikz.net\\/i\\/(.+)\"",0);
	local viewUrl = regex_simple(data,"\\[URL=(.+)\\]",0);
	local  thumbUrl = regex_simple(data, "\\[IMG\\](.+)\\[/IMG\\]",0);
	if(directUrl == "")
		return 0;
	options.setDirectUrl("http://epikz.net/i/"+directUrl);
	options.setViewUrl(viewUrl);
	options.setThumbUrl(thumbUrl);	
	return 1;
}