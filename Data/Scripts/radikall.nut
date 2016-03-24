
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
    local auth_token=null;
	nm.doGet("http://radikall.org/");
    if ( nm.responseCode() == 200) {
        local reg = CRegExp("name=\"auth_token\" value=\"(.+?)\"","i");
		if ( !reg.match(nm.responseBody()) ) {
			return 0;
		}
        auth_token = reg.getMatch(1);
    } else {
        return 0;
    }
    
	nm.setUrl("http://radikall.org/json");
    nm.addQueryParamFile("source",FileName, ExtractFileName(FileName),"");
	nm.addQueryParam("type", "file");
	nm.addQueryParam("action", "upload");
	nm.addQueryParam("privacy", "public");
	nm.addQueryParam("timestamp", time()+"");
	nm.addQueryParam("auth_token", auth_token);
	nm.addQueryParam("category_id", "null");
	nm.addQueryParam("nsfw", "0");

	nm.doUploadMultipartData();
    
    if ( nm.responseCode() == 200) {
        local data = nm.responseBody();
        local t = ParseJSON(data);
        if (t.success.code == 200) {
            local directUrl = t.image.image.url;
            local viewUrl = t.image.url_viewer;
            local thumbUrl = t.image.thumb.url;
            options.setDirectUrl(directUrl);
            options.setViewUrl(viewUrl);
            options.setThumbUrl(thumbUrl);	
            return 1;
        }
        
    }

	return 0;
}