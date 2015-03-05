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

function TrySleep() {
	try {
		sleep(300);
	} catch ( ex ) {
		local retTime = time() + 1;     
		while (time() < retTime);  
	}
}

function readFile(fileName) {
	local myfile = file(fileName,"r");
	local i = 0;
	local res = "";
	while ( !myfile.eos()) {
		res += format("%c", myfile.readn('b'));
		
	}
	return res;
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



function  UploadFile(FileName, options)
{			
	nm.setUrl("http://funkyimg.com/upload/?fileapi" + random());
	nm.addQueryParam("_images", ExtractFileName(FileName));
	nm.addQueryParamFile("images",FileName, ExtractFileName(FileName),"");
	nm.addQueryParam("wmText", "");
	nm.addQueryParam("wmPos", "TOPRIGHT");
	nm.addQueryParam("wmLayout", "2");
	nm.addQueryParam("wmFontSize", "14");
	nm.addQueryParam("addInfoType", "res");
	nm.addQueryParam("labelText", "");

	nm.doUploadMultipartData();
 	local data = nm.responseBody();
	local jid = regex_simple(nm.responseBody(), "jid\" *: *\"(.+)\"", 0);
	if ( jid != "" ) {
		local i = 0;
		while ( true ) {
			local checkUrl = "http://funkyimg.com/upload/check/" + jid + "?_=" + random();
		
			nm.doGet(checkUrl);
			data = nm.responseBody();
			local success = regex_simple(data, "success\" *: *([a-zA-Z]+)", 0);

			if ( success == "true" || success == "True") {
				local thumbUrl = regex_simple(data, "\\[IMG\\](.+)\\[/IMG\\]", 0);
				local directUrl = regex_simple(data, "http:\\/\\/funkyimg\\.com\\/i\\/([a-zA-Z0-9\\.]+)", 0);
				if ( directUrl != "") {
					options.setDirectUrl("http://funkyimg.com/i/" + directUrl);
				}
				options.setThumbUrl(thumbUrl);
				return 1;
			} else if ( success == "" || i > 23 ) {
				break;
			}
			i++;
			TrySleep();
			print("sleeping...");
		}
	}
	return 0;
}