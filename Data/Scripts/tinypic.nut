TinyPicKey <- "00a68ed73ddd54da52dc2d5803fa35ee";
TinyPicID <- "e2aabb8d555322fa";
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

function getSignature(action){
	return  md5 (action + TinyPicID + TinyPicKey);
}
function getUploadKey(){
	local sig = getSignature( "getuploadkey" );
	local url = "http://api.tinypic.com/api.php?action=getuploadkey&tpid="+ TinyPicID +"&sig="+sig+"&responsetype=XML";
	nm.doGet(url);
	local uploadKey = regex_simple(nm.responseBody(),"<uploadkey>(.+)</uploadkey>",0);
	return uploadKey;
}
function  UploadFile(FileName, options)
{	
	local uploadKey = getUploadKey();
	
	nm.setUrl("http://api.tinypic.com/api.php");
	nm.addQueryParam("action", "upload");
	nm.addQueryParam("tpid", TinyPicID);
	nm.addQueryParam("sig", getSignature("upload"));
	nm.addQueryParam("responsetype", "XML");
	nm.addQueryParam("upk", uploadKey);
	nm.addQueryParam("type", "image");
	nm.addQueryParamFile("uploadfile",FileName, ExtractFileName(FileName),"");
	
	nm.doUploadMultipartData();

 	local data = nm.responseBody();
	local directUrl = regex_simple(data, "<fullsize>(.+)</fullsize>" ,0);
	local viewUrl = regex_simple(data, "<emailimcode>(.+)</emailimcode>" ,0);
	local thumbUrl = regex_simple(data, "<thumbnail>(.+)</thumbnail>" ,0);
	viewUrl = reg_replace(viewUrl, "&amp;", "&");
	if(directUrl == "")
		return 0;
	options.setDirectUrl(directUrl);
	options.setViewUrl(viewUrl);
	options.setThumbUrl(thumbUrl);	
	return 1;
}