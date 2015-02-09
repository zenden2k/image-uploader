appKey <- "2fbf2be64367abf2515f8b5494a5bc43";
appSecret <- "116fdf4bcac0cf76";
accessType <- "app_folder";

authStep1Url <- "https://www.flickr.com/services/oauth/request_token";
authStep2Url <- "https://www.flickr.com/services/oauth/access_token";

oauth_token_secret <- "";
oauth_token <- "";

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

function generateNonce() {
	local res = "";
	res += format("%d%d%d", random(2000), random(2000), random(2000));
	return res;
}

function custom_compare(item1,item2){
	local a = item1.a+"="+item1.b;
	local b = item2.a+"="+item2.b;
	if(a>b) return 1;
	else if(a<b) return -1;
	return 0;
}

function openUrl(url) {
	try{
		return ShellOpenUrl(url);
	}catch(ex){}

	system("start "+ reg_replace(url,"&","^&") );
}


function base64Encode(input) {
	local keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    local output = "";
    local chr1, chr2, chr3, enc1, enc2, enc3, enc4;
    local i = 0;
	local len = input.len() ;

    while ( i < len ) {

        chr1 = input[i++];
		if ( i< len) {
			chr2 = input[i++];
		} else {
			chr2 = 0;
		}
		if ( i < len ) {
			chr3 = input[i++];
		} else {
			chr3 = 0;
		}

        enc1 = chr1 >> 2;
        enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
        enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
        enc4 = chr3 & 63;

        if (chr2 == 0) {
            enc3 = enc4 = 64;
        } else if (chr3 == 0) {
            enc4 = 64;
        }
		//print("enc1=" + enc1 + " enc2=" + enc2 + " enc3=" + enc3);
        output = output + format("%c", keyStr[enc1] ) + 
			format ( "%c", keyStr[enc2]) 
			+ format("%c", keyStr[enc3])
			+ format("%c", keyStr[enc4]);
    }

    return output;
}

function signRequest(method, url,params, token,tokenSecret) {
	
	if ( token != "" ) {
		params.append({a="oauth_token",b=token});
	}	
	params.sort(custom_compare);

	local normalizedRequest = "";
	
	normalizedRequest += "&" + url_encode(url);
	local tableLen = params.len();
	local baseString = "";
	for ( local i=0; i< tableLen; i++ ) {
		params[i].b = (params[i].b);
		if ( i > 0 ) {
			baseString += "&" ;
		}
		baseString += params[i].a + "=" + url_encode(params[i].b);
		
	}

	
	normalizedRequest = method+"&"+url_encode(url) + "&" + url_encode(baseString) ;
	

	local oauth_signature = hmac_sha1(appSecret + "&" + tokenSecret, normalizedRequest, true);
	//oauth_signature = base64Encode(oauth_signature);
	params.append({a="oauth_signature",b=oauth_signature});
	params.sort(custom_compare);
	
	local getStr="";
	local authorizationString = "OAuth ";
	for ( local i=0; i< params.len(); i++ ) {
		if ( i!=0) {
			authorizationString += ",";
			 getStr += "&";
		}
		getStr += params[i].a + "=" + url_encode(params[i].b);
	}
	
	local sign = "GET";
	sign += "&" + authStep1Url;
	
	return getStr;
}

function sendOauthRequest(method, url, params, token,tokenSecret) {
	local getStr = signRequest(method,url,params,  token,tokenSecret);


	if ( method == "GET") {
		nm.doGet(url + "?" + getStr);
	} else if ( method == "POST") {
		nm.setUrl(url);
		local tableLen = params.len();

		for ( local i=0; i< tableLen; i++ ) {
			nm.addQueryParam( params[i].a, params[i].b);		
		}
		nm.doPost("");
	}
	return 0;
}
function Login() {
	oauth_token_secret = ServerParams.getParam("oauth_token_secret");
	oauth_token = ServerParams.getParam("oauth_token");
	
	if ( oauth_token_secret != ""  &&  oauth_token != ""){
		return true;
	}
	local email = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	
	local params=[	{a="oauth_consumer_key", b=appKey},
					{a="oauth_signature_method", b="HMAC-SHA1"},
					{a="oauth_timestamp", b=""+time()},
					{a="oauth_version", b="1.0"},
					{a="oauth_nonce", b=generateNonce() },
					{a="oauth_callback", b="oob"}
			  ];
	sendOauthRequest("GET", authStep1Url, params, "", "");
	
	local data =  nm.responseBody();

	local temp_oauth_token_secret = regex_simple(data, "oauth_token_secret=([^&]+)", 0);
	local temp_oauth_token = regex_simple(data, "oauth_token=([^&]+)", 0);
	if ( temp_oauth_token == "" ) {
		return false;
	}
	openUrl("https://www.flickr.com/services/oauth/authorize?oauth_token=" + temp_oauth_token);
	
	msgBox("Please allow Image Uploader to connect with your Flickr account (in web browser) and then press OK.\r\n\r\n"+
		"Пожалуйста, разрешите Image Uploader подключиться к вашему аккаунту Flickr (нажмите кнопку Allow в запущенном браузере), и после этого нажмите OK."
	);
	
	local verificationCode = InputDialog("Please enter verification code:", "");
	local params2=[	{a="oauth_consumer_key", b=appKey},
					{a="oauth_signature_method", b="HMAC-SHA1"},
					{a="oauth_timestamp", b=""+time()},
					{a="oauth_version", b="1.0"},
					{a="oauth_nonce", b=generateNonce() },
					{a="oauth_verifier", b=verificationCode }					
			  ];
	sendOauthRequest("GET", authStep2Url, params2, temp_oauth_token, temp_oauth_token_secret);
	data =  nm.responseBody();
	
	oauth_token_secret = regex_simple(data, "oauth_token_secret=([^&]+)", 0);
	oauth_token = regex_simple(data, "oauth_token=([^&]+)", 0);
	
	if ( oauth_token_secret != "" && oauth_token != "" ) {
		
		ServerParams.setParam("oauth_token_secret", oauth_token_secret);
		ServerParams.setParam("oauth_token", oauth_token);
		return true;
	}
	return false;
}

function isAuthorized() {
	if ( oauth_token_secret != "" && oauth_token != "" ) {
		return true;
	}
	return false;
}

function msgBox(text) {
	try {
		DebugMessage(text, false);
		return true;
	}catch(ex) {
	}
	local tempScript = "%temp%\\imguploader_msgbox.vbs";
	system("echo Set objArgs = WScript.Arguments : messageText = objArgs(0) : MsgBox messageText > \"" + tempScript + "\"");
	system("cscript \"" + tempScript + "\" \"" + text + "\"");
	system("del /f /q \"" + tempScript + "\"");
	
	
	return true;
}
function  UploadFile(FileName, options) {		

	if (!Login() ) {
		return 0;
	}
	local userPath = ServerParams.getParam("UploadPath");
	if ( userPath!="" && userPath[userPath.len()-1] != "/") {
		userPath+= "/";
	}
	local path = "sandbox/" + userPath + ExtractFileName(FileName);
	
	local uploadUrl = "https://up.flickr.com/services/upload/";
	local albumId = options.getFolderID();

	
	nm.setUrl(uploadUrl);
	local params=[	{a="oauth_consumer_key", b=appKey},
					{a="oauth_signature_method", b="HMAC-SHA1"},
					{a="oauth_timestamp", b=""+time()},
					{a="oauth_version", b="1.0"},
					{a="oauth_nonce", b=generateNonce() },
					{a="oauth_callback", b="oob"}
			  ];
	signRequest("POST", uploadUrl, params, oauth_token, oauth_token_secret);
	nm.addQueryParamFile("photo", FileName, ExtractFileName(FileName), "");
	local tableLen = params.len();

	for ( local i=0; i< tableLen; i++ ) {
		nm.addQueryParam( params[i].a, params[i].b);		
	}
	
	nm.doUploadMultipartData();
	local data = nm.responseBody();
	local photoid = regex_simple(data, "<photoid>(.+)</photoid>", 0);

	nm.doGet("https://api.flickr.com/services/rest/?method=flickr.photos.getSizes&api_key=" + appKey + "&photo_id="+photoid);
	
	data = nm.responseBody();
	local originalPhoto = regex_simple(data, "label=\"Original\".+source=\"(.+)\"", 0);
	local thumbUrl = regex_simple(data, "label=\"Thumbnail\".+source=\"(.+)\"", 0);
	
	if ( originalPhoto != "") {

		options.setDirectUrl(originalPhoto);
		options.setThumbUrl(thumbUrl);
		
		nm.doGet("https://api.flickr.com/services/rest/?method=flickr.photos.getInfo&api_key=" + appKey +"&photo_id=" + photoid +"&oauth_token="+oauth_token);
		data = nm.responseBody();
		local photoPage  = regex_simple(data, "type=\"photopage\">(.+)<", 0);
		options.setViewUrl(photoPage );
		
		if ( albumId != "") {
			
			local params3=[	{a="oauth_consumer_key", b=appKey},
				{a="oauth_signature_method", b="HMAC-SHA1"},
				{a="oauth_timestamp", b=""+time()},
				{a="oauth_version", b="1.0"},
				{a="oauth_nonce", b=generateNonce() },
				{a="api_key", b=appKey},
				{a="method", b="flickr.photosets.addPhoto"},
				{a="photoset_id", b=albumId},
				{a="photo_id", b=photoid}
			];
			  
			sendOauthRequest("POST", "https://api.flickr.com/services/rest/", params3, oauth_token, oauth_token_secret);
		}
		return 1;
	}
	
	return 0;
}

function GetFolderList(list)
{
	if (!Login() ) {
		return 0;
	}
	
	local params=[	{a="oauth_consumer_key", b=appKey},
					{a="oauth_signature_method", b="HMAC-SHA1"},
					{a="oauth_timestamp", b=""+time()},
					{a="oauth_version", b="1.0"},
					{a="oauth_nonce", b=generateNonce() },
					{a="api_key", b=appKey},
					{a="method", b="flickr.photosets.getList"}
			  ];
			  
	sendOauthRequest("GET", "https://api.flickr.com/services/rest/", 
	params, oauth_token, oauth_token_secret);
	local data = nm.responseBody();
	//DebugMessage(data);
	local start = 0;
	while(1)
	{
		local title="",id="",summary="";
		local ex = regexp("<photoset ");
		local res = ex.search(data, start);
		local link = "";
		local album = CFolderItem();
		if(res != null){	
			start = res.end;
		}
		else break;
		
		id = regex_simple(data,"id=\"(\\d+)\"",start);
		title = regex_simple(data,"<title>(.+)</title>",start);
		summary = regex_simple(data,"<description>(.+)</description>",start);
		
		album.setId(id);
		album.setTitle(title);
		album.setSummary(summary);
		//album.setViewUrl(link);
		//album.setParentId(parentid);

		list.AddFolderItem(album);
		
	}
	
	return 1; //success
}

function reg_replace(str, pattern, replace_with)
{
	local res = str.find(pattern);
		
	if(res != null){	
		return str.slice(0,res) +replace_with+ str.slice(res + pattern.len());
	}
	return str;
}

function hex2int(str){
	local res = 0;
	local step = 1;
	for( local i = str.len() -1; i >= 0; i-- ) {
		local val = 0;
		local ch = str[i];
		if ( ch >= 'a' && ch <= 'f' ) {
			val = 10 + ch - 'a';
		}
		else if ( ch >= '0' && ch <= '9' ) {
			val = ch - '0';
		}
		res += step * val;
		step = step * 16;
	}
	return res;
}

function unescape_json_string(data) {
    local tmp;

    local ch = 0x0424;
	local result = data;
	local ex = regexp("\\\\u([0-9a-fA-F]{1,4})");
	local start = 0;
	local res = null;
	for(;;) {
		res = ex.capture(data, start);
		local resultStr = "";
		if (res == null){
			break;
		}
			
		resultStr = data.slice(res[1].begin, res[1].end);
		ch = hex2int(resultStr);
		start = res[1].end;
		 if(ch>=0x00000000 && ch<=0x0000007F)
			tmp = format("%c",(ch&0x7f));
		else if(ch>=0x00000080 && ch<=0x000007FF)
			tmp = format("%c%c",(((ch>>6)&0x1f)|0xc0),((ch&0x3f)|0x80));
		else if(ch>=0x00000800 && ch<=0x0000FFFF)
		   tmp= sprintf("%c%c%c",(((ch>>12)&0x0f)|0xe0),(((ch>>6)&0x3f)|0x80),((ch&0x3f)|0x80));
			result = reg_replace( result, "\\u"+resultStr, tmp);
   
	}

    return result;
}

function GetServerParamList()
{
	local a =
	{
		oauth_token_secret = "oauth_token_secret"
		oauth_token = "oauth_token"
	}
	return a;
}