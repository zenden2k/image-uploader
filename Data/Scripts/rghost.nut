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
	nm.doGet("http://rghost.net/multiple/upload_host");
	local serverData = nm.responseBody();
	
	local serverId = regex_simple(serverData,"upload_host\":\"(.+)\"",0);
	local token = regex_simple(serverData,"authenticity_token\":\"(.+)\"",0);

	nm.setUrl("http://"+serverId+"/files");
	nm.addQueryParam("authenticity_token",token);
	nm.addQueryParamFile("file",FileName, ExtractFileName(FileName),"");
	nm.doUploadMultipartData();

 	local data = nm.responseBody();

	local ex = regexp("id=\"editfile\"");
	local res = ex.capture(data, 0);
	local resultStr = "";
	if(res == null){	
		return 0;
		
	}
	
	local action=regex_simple(data,"action=\"([^\"]+)\"",res[0].end);
	if ( action == "" ) {
		print("Cannot find action");
		return 0;
	}

	nm.setUrl("http://rghost.net/" + action);
	/*nm.addQueryHeader("Referer","http://rghost.net/"+fileID);
	nm.addQueryHeader("Origin","http://rghost.net/");*/

	nm.addQueryParam("utf8","✓");
	nm.addQueryParam("_method","put");
	nm.addQueryParam("authenticity_token", token);

	nm.addQueryParam("download_url","http://rghost.net"+action);
	nm.addQueryParam("fileset[tags]","");
	nm.addQueryParam("fileset[description]","");
	nm.addQueryParam("fileset[removal_code]","");
	nm.addQueryParam("fileset[password]","");
	nm.addQueryParam("fileset[lifespan]","30");
	nm.addQueryParam("fileset[public]","0");
	nm.addQueryParam("commit","Обновить");


	nm.doPost("");

	data = nm.responseBody();
	local fileID = regex_simple(data,"http:\\/\\/rghost\\.\\w+\\/download\\/(private\\/\\w+\\/\\w+)",0);
	if ( fileID == "" ) {
		fileID = regex_simple(data,"http:\\/\\/rghost\\.\\w+\\/download\\/([A-Za-z0-9_]+)",0);
		}
	local ex = regexp("\\[img\\](.+)\\[/img\\]");
	local res = ex.capture(data, 0);
	local directUrl = "";
	local thumbUrl = "";
	if(res != null){	
		thumbUrl = data.slice(res[1].begin, res[1].end);
		local res2 = ex.capture(data, res[1].begin);
		directUrl = data.slice(res2[1].begin, res2[1].end);
	}	

	//DebugMessage(data, true);
	options.setDirectUrl(directUrl);
	options.setViewUrl("http://rghost.ru/" + fileID);
	options.setThumbUrl(thumbUrl);
	if ( fileID != "" || directUrl != "" ) {	
	return 1;
} else {
	return 0;
}
}