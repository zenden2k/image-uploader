// Script can be reused by the same thread. It means that 
// the same auth cookies will be sent to the server.
loggedIn <- false; // Global flag 


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

function _DoLogin() {
    local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
    if (login == "") {
        return 1;
    }
    
    if (loggedIn){
        return 1;
    }
    
    nm.setUrl("http://forum.funkysouls.com/index.php?s=&act=Login&CODE=01");
    nm.setReferer("https://funkyimg.com/");
    nm.addQueryParam("UserName", login);
    nm.addQueryParam("PassWord", pass);
    nm.setCurlOptionInt(52, 0); //disable CURLOPT_FOLLOWLOCATION
    nm.doPost("");
    if (nm.responseCode() == 302) {
        local funcName = "jQuery" + random()%100000 + "_" + random()%100000; 
        nm.doGet("https://forum.funkysouls.com/whoami/?callback="+ funcName + "&_=" + random() % 100000);
        if (nm.responseCode() == 200) {
            local reg = CRegExp("jQuery\\d+_\\d+\\((.+?)\\)", "");
            if ( reg.match(nm.responseBody()) ) {
                local jsonTxt = reg.getMatch(1);
                local t = ParseJSON(jsonTxt);
                if ("name" in t && "ukey" in t) {
                    nm.setUrl("https://funkyimg.com/user/auth");
                    nm.setReferer("https://funkyimg.com/");
                    nm.addQueryParam("name", t.name);
                    nm.addQueryParam("ukey", t.ukey); 
                    nm.doPost("");
                    if(nm.responseCode() == 200) {
                        local s = ParseJSON(nm.responseBody());
                        if ( "status" in s && s.status == true) {
                            loggedIn = true;
                            return 1;
                        }
                    }
                }  
            }
        }  
    }
    return 0;
    
}

function DoLogin() {
    if (!Sync.beginAuth()) {
        return 0;
    }
    local res = _DoLogin();
    Sync.endAuth();
    return res;
}

function  UploadFile(FileName, options)
{			
    if (!DoLogin()) {
        return 0;
    }
	nm.setUrl("https://funkyimg.com/upload/?fileapi" + random());
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
			local checkUrl = "https://funkyimg.com/upload/check/" + jid + "?_=" + random();
		
			nm.doGet(checkUrl);
			data = nm.responseBody();
			local success = regex_simple(data, "success\" *: *([a-zA-Z]+)", 0);

			if ( success == "true" || success == "True") {
				local thumbUrl = regex_simple(data, "\\[IMG\\](.+)\\[/IMG\\]", 0);
				local directUrl = regex_simple(data, "https:\\/\\/funkyimg\\.com\\/i\\/([a-zA-Z0-9\\.]+)", 0);
				if ( directUrl != "") {
					options.setDirectUrl("https://funkyimg.com/i/" + directUrl);
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