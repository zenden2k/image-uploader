appKey <- "973quph3jxdgqoe";
appSecret <- "wloizpn331cc8zd";
accessType <- "app_folder";

authStep1Url <- "https://api.dropbox.com/1/oauth/request_token";
authStep2Url <- "https://api.dropbox.com/1/oauth/access_token";

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

function getTimeStamp() {
	local days = ["Sun","Mon","Tue","Wed","Thu", "Fri", "Sat"];
	local months = ["Jan","Feb","Mar","Apr","May","Jun","Jul", "Aug","Sep","Oct", "Nov","Dec"];
	local dt = date(time(),'u');
	local res = format("%s, %d %s %d %02d:%02d:%02d +0000",days[dt.wday],dt.day,months[dt.month],dt.year,dt.hour,dt.min,dt.sec );
	//print(res);
	return res;
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


function _WriteLog(type,message) {
	try {
		WriteLog(type, message);
	} catch (ex ) {
		print(type + " : " + message);
	}
}

function signRequest(url, token,tokenSecret) {
	local params=[	{a="oauth_consumer_key", b=appKey},
					{a="oauth_signature_method", b="PLAINTEXT"},
					{a="oauth_timestamp", b=getTimeStamp()},
					{a="oauth_version", b="1.0"},
					{a="oauth_nonce", b=generateNonce() }
			  ];
	if ( token != "" ) {
		params.append({a="oauth_token",b=token});
	}	
	params.sort(custom_compare);

	local normalizedRequest = "POST";
	local tableLen = params.len();
	for ( local i=0; i< tableLen; i++ ) {
		params[i].b = nm.urlEncode(params[i].b);
		normalizedRequest += "&" + params[i].a + "=" + params[i].b;
		
	}
	//print("\r\n"+normalizedRequest);
	local oauth_signature = /*sha1(normalizedRequest)*/appSecret + "&" + tokenSecret;
	params.append({a="oauth_signature",b=oauth_signature});
	params.sort(custom_compare);
	
	local getStr="";
	local authorizationString = "OAuth ";
	for ( local i=0; i< params.len(); i++ ) {
		if ( i!=0) {
			authorizationString += ",";
			 getStr += "&";
		}
		authorizationString += params[i].a + "=\"" + params[i].b +"\"";
		getStr += params[i].a + "="+params[i].b;
	}
	
	//print("\r\n"+oauth_signature);
	//print("\r\n"+authorizationString);
	
	local sign = "POST";
	sign += "&" + authStep1Url;
	nm.addQueryHeader("Authorization", authorizationString);
	
	return getStr;
}

function sendOauthRequest(url, token,tokenSecret) {
	nm.setUrl(url);
	signRequest(url, token,tokenSecret);
	nm.doPost("" );
	return 0;
}
function openUrl(url) {
	try{
		return ShellOpenUrl(url);
	}catch(ex){}

	system("start "+ reg_replace(url,"&","^&") );
}

function DoLogin() {
	oauth_token_secret = ServerParams.getParam("oauth_token_secret");
	oauth_token = ServerParams.getParam("oauth_token");
	
	if ( oauth_token_secret != ""  &&  oauth_token != ""){
		return 1;
	}
	local email = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	
	sendOauthRequest(authStep1Url, "", "");
	
	local data =  nm.responseBody();
	local temp_oauth_token_secret = regex_simple(data, "oauth_token_secret=([^&]+)", 0);
	local temp_oauth_token = regex_simple(data, "oauth_token=([^&]+)", 0);
	
	openUrl("https://www.dropbox.com/1/oauth/authorize?oauth_token=" + temp_oauth_token);
	
	msgBox("Please allow Image Uploader to connect with your Dropbox (in web browser) and then press OK.\r\n\r\n"+
		"Пожалуйста, разрешите Image Uploader подключиться к вашему аккаунту Dropbox (нажмите кнопку Allow в запущенном браузере), и после этого нажмите OK."
	);
	
	sendOauthRequest(authStep2Url, temp_oauth_token, temp_oauth_token_secret);
	data =  nm.responseBody();
	oauth_token_secret = regex_simple(data, "oauth_token_secret=([^&]+)", 0);
	oauth_token = regex_simple(data, "oauth_token=([^&]+)", 0);
	
	if ( oauth_token_secret != "" && oauth_token != "" ) {
		ServerParams.setParam("oauth_token_secret", oauth_token_secret);
		ServerParams.setParam("oauth_token", oauth_token);
		
		/*local url = "https://api.dropbox.com/1/account/info";
		signRequest(url, oauth_token,oauth_token_secret);
		nm.doGet(url);
		data =  nm.responseBody();
		local displayName = regex_simple(data, "display_name\": \"(.+)\",", 0);
		ServerParams.setParam("Login", displayName);
		*/
		return 1;
	}
	return 0;
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

function min(a,b) {
	return a < b ? a : b;
}
function  UploadFile(FileName, options) {		

	if (!DoLogin() ) {
		return 0;
	}
	local url = null;
	local userPath = ServerParams.getParam("UploadPath");
	if ( userPath!="" && userPath[userPath.len()-1] != "/") {
		userPath+= "/";
	}
	local chunkSize = (50*1024*1024).tofloat();
	local fileSize = 0;
	try { 
		GetFileSize(FileName);
	} catch ( ex ) {
		
	}
	
	if ( fileSize < 0 ) {
		_WriteLog("error","fileSize < 0 ");
		return 0;
	}
	local path = "sandbox/" + userPath + ExtractFileName(FileName);
	if ( fileSize > 150000000 ) {
		local chunkCount = ceil(fileSize / chunkSize);
		local upload_id = null;
		local offset = 0;
		for(local i = 0; i < chunkCount; i++ ) {
			for ( local j =0; j < 2; j++ ) {
				try {
					nm.setChunkOffset(offset.tofloat());
				} catch ( ex ) {
					_WriteLog("error", "Your Image Uploader version does not support chunked uploads for big files. \r\nPlease update to the latest version");
					return 0;
				}
				
				local chunkSize = min(chunkSize,fileSize.tofloat()-offset);
				nm.setChunkSize(chunkSize);
				url = "https://api-content.dropbox.com/1/chunked_upload?overwrite=false&offset="+offset ;
				if ( upload_id ) {
					url += "&upload_id="+nm.urlEncode(upload_id);
				}
				signRequest(url, oauth_token,oauth_token_secret);
				nm.setUrl(url);
				
				nm.setMethod("PUT");
				nm.doUpload(FileName,"");
				if ( nm.responseCode() != 200 ) {
					_WriteLog("warning","Chunk upload failed, offset="+offset+", size="+chunkSize+(j< 1? "Trying again..." : ""));
					if ( nm.responseCode() == 403 ) {
						_WriteLog("error","Upload failed. Access denied");
						return 0;
					}
				} else {
					local t = ParseJSON(nm.responseBody());
					offset = t.offset;
					upload_id = t.upload_id;
					break;
				}
			}
			//return 0;
		}
		if ( !upload_id ) {
			_WriteLog("error","Upload failed");
			return 0;
		}
		url = "https://api-content.dropbox.com/1/commit_chunked_upload/auto/" + path;
		
		nm.setUrl(url);
		signRequest(url, oauth_token,oauth_token_secret);
		nm.setMethod("POST");
		nm.doPost("upload_id="+upload_id);
		//WriteLog("error",nm.responseBody());
		if ( nm.responseCode() != 200 ) {
			_WriteLog("error",nm.responseCode().tostring());
		}

	} else {
		url = "https://api-content.dropbox.com/1/files/" + path ;
		local getParams = signRequest(url, oauth_token,oauth_token_secret);

		nm.setUrl(url);
		nm.addQueryParamFile("file",FileName, ExtractFileName(FileName),"");
		nm.addQueryParam("overwrite", "false");
		nm.doUploadMultipartData();
	}

	local data = nm.responseBody();

	path = regex_simple(data, "path\": \"(.+)\",", 0);
	path = unescape_json_string(path);
	local thumbExists = regex_simple(data, "thumb_exists\": \"(.+)\",", 0);
	
	url = "https://api.dropbox.com/1/shares/sandbox" +path;
	signRequest(url, oauth_token,oauth_token_secret);
	nm.setUrl(url);
	nm.doPost("");
	local data =  nm.responseBody();

	local viewUrl = unescape_json_string(regex_simple(data, "url\": \"(.+)\",", 0));
	options.setViewUrl(viewUrl );

	url = "https://api.dropbox.com/1/media/sandbox" + path;
	signRequest(url, oauth_token,oauth_token_secret);
	nm.setUrl(url);
	nm.doPost("");
	
	local directUrl = regex_simple(nm.responseBody(), "url\": \"(.+)\",", 0);
	//options.setDirectUrl(directUrl );
	
	if ( url != "" ) {
		return 1;
	}
 	
	return 0;
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
		   tmp= format("%c%c%c",(((ch>>12)&0x0f)|0xe0),(((ch>>6)&0x3f)|0x80),((ch&0x3f)|0x80));
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
		UploadPath = "Upload Path"
	}
	return a;
}