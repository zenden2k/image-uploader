function str_replace(str, pattern, replace_with)
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

function  UploadFile(FileName, options)
{		
	nm.doGet("http://takebin.com");
	if ( nm.responseCode() != 200 ) {
		return 0;
	}
	local reg = CRegExp("\\[\"token\"\\]\\s*=\\s*'(.+?)'","");
	if ( !reg.match(nm.responseBody()) ) {
		return 0;
	}
	local token = reg.getMatch(0);
	nm.setUrl("http://takebin.com/action");
	nm.addQueryParamFile("file[]",FileName, ExtractFileName(FileName),"");
	nm.addQueryParam("fileName[]", ExtractFileName(FileName));
	nm.addQueryParam("fileDesc[]", "");
	nm.addQueryParam("filePass[]", "");
	nm.addQueryParam("fileTwit[]", "undefined");
	nm.addQueryParam("filePrivate[]", "undefined");
	nm.addQueryParam("fileExpire", "D");
	nm.addQueryParam("fileFolder", "undefined");
	nm.addQueryParam("token", token);
	nm.addQueryParam("type", "upload");
	nm.doUploadMultipartData();
	if ( nm.responseCode() != 200 )  {
		return 0;
	}
 	local data = nm.responseBody();
	local t = ParseJSON(data);
	if ( t == null ) {
		return 0;
	}
	
	options.setViewUrl("http://takebin.com" + t[0].fileUrl);
	return 1;
}