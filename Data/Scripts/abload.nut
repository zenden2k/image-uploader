userName <- "";

function Login() {
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	
    nm.setUserAgent("Abloadlib/0.1");
	nm.doGet( "https://abload.de/");
	
	nm.setUrl( "https://abload.de/login.php");
	nm.addQueryParam( "name", login );
	nm.addQueryParam( "password", pass );
	//nm.addQueryParam( "login_remember", "on" );
	nm.addQueryParam( "cookie", "on" );
    nm.setCurlOptionInt(52, 0); //disable CURLOPT_FOLLOWLOCATION 
	nm.doPost("");

    if(nm.responseCode() == 302 || nm.responseCode() == 200 ){
        nm.doGet("https://abload.de/calls/userXML.php");
        if ( nm.responseCode() == 200) {
            local data = nm.responseBody();
            local xml = SimpleXml();
            if(xml.LoadFromString(data)) {
                local root = xml.GetRoot("info", false);
                local userNode = root.GetChild("user", false);
                local userId = userNode.Attribute("id");
                userName = userNode.Attribute("name");
                local n = userNode.GetChildCount();
                local salt = "";
                for ( local i = 0; i< n;  i++) {
                    local node = userNode.GetChildByIndex(i);
                    if ( node.Name() == "setting" && node.Attribute("name") == "salt") {
                        salt = node.Attribute("value");
                    }
                }
                
                return 1;
            }
        }
        
    } 
    
    WriteLog("error", "abload.de: Authentication failed using login '" + login + "'");
	return 0;
}

function UploadFile(FileName, options) {
    if (userName == "" && !Login()) {
        return 0;
    }
    
	local name = ExtractFileName(FileName);
	local mime = GetFileMimeType(name);
	nm.setUrl("https://abload.de/upload.php");//?printr=true
    nm.addQueryParam("resize", "none");
    nm.addQueryParam("gallery", "NULL");
    nm.addQueryParamFile("img0", FileName, name, mime);
	nm.doUploadMultipartData();
    
    local html = nm.responseBody();
    local doc = Document(html);
    local form = doc.find("form");
    
    if ( form.length() ) {
        local url = form.attr("action");
        
        if (url.slice(0, 2) == "//" ) {
            url = "https:" + url;
        }

        nm.setUrl(url);
        form.find("input").each( function(index,elem) {    
            nm.addQueryParam(elem.attr("name"), elem.attr("value"));  
        });
        form.find("textarea").each( function(index,elem) {    
            nm.addQueryParam(elem.attr("name"), elem.text());  
        });
        nm.doPost("");
        if ( nm.responseCode() == 200) {
            local data = nm.responseBody();
            local reg = CRegExp("Direktlink.+?value=\"(https?://abload\\.de/img/.+?)\"", "smi");
            local directUrl = "";
            local viewUrl = "";
            local thumbUrl = "";

			if ( reg.match(data) ) {
				directUrl = reg.getMatch(1);
                options.setDirectUrl(directUrl);
			}
            local reg2 = CRegExp("\\[url=(.+?)\\]\\[img\\](.+?)\\[/img\\]", "mi");
            if ( reg2.match(data) ) {
				viewUrl = reg2.getMatch(1);
				thumbUrl = reg2.getMatch(2);
                options.setViewUrl(viewUrl);
                options.setThumbUrl(thumbUrl);
            }
            if (directUrl != "" || viewUrl != "") {
                return 1;
            }        
        }
        
    }
    return 0;
}