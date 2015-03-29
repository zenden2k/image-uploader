
clientId <- "4851603";
redirectUri <- "https://oauth.vk.com/blank.html";
redirectUrlEscaped <- "https:\\/\\/oauth\\.vk\\.com\\/blank\\.html";
apiVersion <- "5.29";
token <- "";
expiresIn <- 0;
userId <- 0;

function tr(key, text) {
	try {
		return Translate(key, text);
	}
	catch(ex) {
		return text;
	}
}

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

function OnUrlChangedCallback(data) {
	local reg = CRegExp("^" +redirectUrlEscaped, "");
	if ( reg.match(data.url) ) {
		local br = data.browser;
		DebugMessage(br.getDocumentBody(), true);
		local regError = CRegExp("error=([^&]+)", "");
		if ( regError.find(data.url) ) {
			DebugMessage("error="+ regError.getMatch(0));
		}
		local regToken = CRegExp("access_token=([^&]+)", "");
		if ( regToken.find(data.url) ) {
			token = regToken.getMatch(0);
			DebugMessage("token="+ token);
		}
		
		local regExpires = CRegExp("expires_in=([^&]+)", "");
		if ( regExpires.find(data.url) ) {
			expiresIn = regExpires.getMatch(0);
			DebugMessage("expiresIn="+ expiresIn);
		}
		
		local regUserId = CRegExp("user_id=([^&]+)", "");
		if ( regUserId.find(data.url) ) {
			userId = regUserId.getMatch(0);
			DebugMessage("userId="+ userId);
		}

		
	}
	//DebugMessage(data.url);
}

function OnLoadFinished(data) {

	//DebugMessage("Title=" + br.title());
}

function OnNavigateError(data) {
	//DebugMessage(data.statusCode.tostring());
}
function  UploadFile(FileName, options)
{
	local browser = CWebBrowser();
	browser.setTitle(tr("vk.browser.title", "Vk.com authorization"))
	browser.setOnUrlChangedCallback(OnUrlChangedCallback, null);
	browser.setOnNavigateErrorCallback(OnNavigateError, null);
	browser.setOnLoadFinishedCallback(OnLoadFinished, null);
	browser.navigateToUrl("http://zenden");
		//browser.navigateToUrl("http://detectmybrowser.com/");
	
	local url = "https://oauth.vk.com/authorize?" + 
			"client_id=" + clientId  + 
			"&scope=photos" +
			"&redirect_uri=" + nm.urlEncode(redirectUri) +  
			"&display=popup" + 
			"&v=" + apiVersion  + 
			"&response_type=token";
	DebugMessage(url);
	browser.navigateToUrl(url
	);
	browser.showModal();
	return 1;
}