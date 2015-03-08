token <- "";
tokenType <- "";
login <- "";
enableOAuth <- true;

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
	local start = 0;

	while( (res = resultStr.find(pattern,start)) != null ) {	

		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
		start = res + replace_with.len();
	}
	return resultStr;
}



function _WriteLog(type,message) {
	try {
		WriteLog(type, message);
	} catch (ex ) {
		print(type + " : " + message);
	}
}

function checkResponse() {
	if ( nm.responseCode() == 0 || (nm.responseCode() >= 400 && nm.responseCode() <= 499)) {
		_WriteLog("error", "Response code " + nm.responseCode() + "\r\n" + nm.errorString() );
		return 0;
	}
	return 1;
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

function openUrl(url) {
	try{
		return ShellOpenUrl(url);
	}catch(ex){}

	system("start "+ reg_replace(url,"&","^&") );
}

function inputBox(prompt, title) {
	try {
		return InputDialog(prompt, "");
	}catch (e){}
	local tempScript = "%temp%\\imguploader_inputbox.vbs";
	prompt = reg_replace(prompt, "\n", "\" ^& vbCrLf ^& \"" );
	local tempOutput = getenv("TEMP") + "\\imguploader_inputbox_output.txt";
	local command = "echo result = InputBox(\""+ prompt + "\", \""+ title + "\") : Set objFSO=CreateObject(\"Scripting.FileSystemObject\") : Set objFile = objFSO.CreateTextFile(\"" + tempOutput + "\",True) : objFile.Write result : objFile.Close  > \"" + tempScript + "\"";
	system(command);
	command = "cscript /nologo \"" + tempScript + "\"";// > \"" + tempOutput + "\"";*/
	system(command);
	local res = readFile(tempOutput);
	system("rm \""+ tempOutput + "\"");
	return res;
}

function getAuthorizationString() {
	if ( tokenType == "oauth") {
		return "OAuth " + token;
	}
	return "FimpToken realm=\"fotki.yandex.ru\", token=\""+token+"\"";
}




function DoLogin() 
{ 
	if ( enableOAuth ) {
		token = ServerParams.getParam("token");
	    	tokenType = ServerParams.getParam("tokenType");
		if ( token != "" && ServerParams.getParam("PrevLogin") == ServerParams.getParam("Login") ) {
			if ( tokenType == "oauth" ) {
				local OAuthLogin = ServerParams.getParam("OAuthLogin");
				if ( OAuthLogin != "") {
					login = OAuthLogin;
				}
			}
			return 1;
		}
		openUrl("https://oauth.yandex.ru/authorize?response_type=code&client_id=7e20041e4444421a8d3df62bf312acfc");
		
	    	local confirmCode = inputBox("You need to need to sign in to your Yandex.Fotki account in web browser which just have opened and then copy confirmation code into the text field below. Please enter confirmation code:", "Image Uploader - Enter confirmation code");
		if ( confirmCode != "" ) {	
			nm.setUrl("https://oauth.yandex.ru/token");
			nm.addQueryParam("grant_type", "authorization_code");
			nm.addQueryParam("code", confirmCode);
			nm.addQueryParam("client_id", "7e20041e4444421a8d3df62bf312acfc");
			nm.addQueryParam("client_secret", "fed316382d3e4bcda82903382a8d00c0");
			nm.doPost("");
			if ( !checkResponse() ) {
				return 0;
			}
				
			local accessToken = regex_simple(nm.responseBody(), "access_token\": \"(.+)\",", 0);
			if ( accessToken != "" ) {
				
				token = 	accessToken;
				tokenType = "oauth";
				ServerParams.setParam("token", token);
				ServerParams.setParam("tokenType", tokenType);
				
				if ( tokenType == "oauth" ) {
					nm.addQueryHeader("Authorization", getAuthorizationString());
					nm.addQueryHeader("Expect", "");
					nm.addQueryHeader("Connection", "close");
					nm.doGet("http://api-fotki.yandex.ru/api/me/");
						
					if ( !checkResponse() ) {
						return 0;
					}
					
					login = regex_simple(nm.responseBody(),"http:\\/\\/api-fotki.yandex.ru\\/api\\/users\\/(.+)\\/albums\\/",0);
					ServerParams.setParam("PrevLogin", ServerParams.getParam("Login"));
					ServerParams.setParam("OAuthLogin", login);
				} 
		
			
				return 1;
			} else {
				print("Unable to get OAuth token!");
				return 0;
			}
		}
	}
	
 	
	login <- ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");

	if(login == "" || pass=="")
	{
		print("E-mail and password should not be empty!");
		return 0;
	}
	nm.addQueryHeader("Expect","");
	nm.setUrl("http://auth.mobile.yandex.ru/yamrsa/key/");
	nm.doGet("");
	local data = nm.responseBody();
	local reqid="";
	local publicKey="";
	
	reqid = regex_simple(data,"<request_id>(.+)</request_id>",0);
	
	publicKey = regex_simple(data,"<key>(.+)</key>",0);
		
	nm.setUrl("http://auth.mobile.yandex.ru/yamrsa/token/");
	nm.addQueryHeader("Expect","");
	nm.addQueryParam("request_id", reqid);

	local credentials = "<credentials login=\""+login+"\" password=\""+pass+"\"/>";
	local encrypted = YandexRsaEncrypter(publicKey, credentials);
	nm.addQueryParam("credentials", encrypted);

	nm.doPost("");
	data = nm.responseBody();
  	
  
	token = regex_simple(data,"<token>(.+)</token>",0);
  
	if(token == "")
	{
		print("Authentication failed for username '"+login +"'");
		return 0;
	}
	ServerParams.setParam("tokenType", "");
	return 1; //Success login
} 

function internal_parseAlbumList(data,list,parentid)
{
	local start = 0;
	
	while(1)
	{
		local title="",id="",summary="";
		local ex = regexp("<entry");
		local res = ex.search(data, start);
		local link = "";
		local album = CFolderItem();
		if(res != null){	
			start = res.end;
		}
		else break;
		
		id = regex_simple(data,"album:(\\d+)</id>",start);
		title = regex_simple(data,"<title>(.+)</title>",start);
		summary = regex_simple(data,"<summary>(.+)</summary>",start);
		link = regex_simple(data,"link href=[\",'](.+)[\",'] rel=[\",']alternate[\",']",start);
		
		album.setId(id);
		album.setTitle(title);
		album.setSummary(summary);
		album.setViewUrl(link);
		album.setParentId(parentid);
		list.AddFolderItem(album);
		
	}
}

function internal_loadAlbumList(list)
{
	
	nm.addQueryHeader("Authorization", getAuthorizationString());
	nm.addQueryHeader("Expect", "");
	nm.addQueryHeader("Connection", "close");

	nm.setUrl("http://api-fotki.yandex.ru/api/users/"+login+"/albums/");

	nm.doGet("");
	
	internal_parseAlbumList(nm.responseBody(), list,"");
}

function GetFolderList(list)
{
	if(token == "")
	{
		if(!DoLogin())
			return 0;
	}

	internal_loadAlbumList(list);
	return 1; //success
}

function CreateFolder(parentAlbum,album)
{
	local title =album.getTitle();
	local summary = album.getSummary();
	local accessType = album.getAccessType();
	local parentId = album.getParentId; 
	if(token == "")
	{
		DoLogin();
	}

	nm.addQueryHeader("Authorization",getAuthorizationString());
  	nm.setUrl("http://api-fotki.yandex.ru/api/users/"+login+"/albums/");
	nm.addQueryHeader("Content-Type", "application/atom+xml; charset=utf-8; type=entry");
	nm.addQueryHeader("Connection", "close");
	local data = "<entry xmlns=\"http://www.w3.org/2005/Atom\" xmlns:f=\"yandex:fotki\">\r\n"+
	"<title>"+title+"</title>\r\n"+
	"<summary>"+summary+"</summary>\r\n"+
	"</entry>";
	
	nm.doPost(data);

	local response = nm.responseBody();
	
	local id = regex_simple(response,"album:(\\d+)</id>",0);
		
	album.setId(id);
	if(nm.responseCode() != 201) // HTTP Status code 201 = Created
		return 0;
	
	return 1;
}

function  UploadFile(FileName, options)
{
   if(token == "")
	{
		if(!DoLogin())
			return 0;
	}

	nm.addQueryHeader("Authorization",getAuthorizationString());
  
	local url="";
	local albumStr = options.getFolderID();
	
	if(albumStr!="")
		url = "http://api-fotki.yandex.ru/api/users/"+login+"/album/"+albumStr+"/photos/";
	else
		url = "http://api-fotki.yandex.ru/api/users/"+login+"/photos/";
	
	nm.setUrl(url);
	
	local ServerFileName = options.getServerFileName();
	if(ServerFileName=="") 
	   ServerFileName = ExtractFileName(FileName);
	local 	encodedFname = nm.urlEncode(ServerFileName);
	nm.addQueryHeader("Slug",encodedFname);
	nm.addQueryHeader("Connection", "close");
	nm.addQueryHeader("Content-Type", GetFileMimeType( FileName ) );
	nm.addQueryHeader("Expect", "");
	
	nm.doUpload(FileName, "");
	
	if(nm.responseCode() != 201) 
	{
		print(nm.responseBody());
		return 0;
	}
	
	local data = nm.responseBody();
	local directUrl="";
	local thumbUrl ="";
	
	local tempUrl = regex_simple(data,"content src=\"(.+)\"",0);
	local viewUrl = regex_simple(data,"link href=\"(.+)\" rel=\"alternate\"",0);
	  
	directUrl = reg_replace(tempUrl,"_XL","_orig")+"."+GetFileExtension(FileName);
	thumbUrl = reg_replace(tempUrl,"_XL","_M")+"."+GetFileExtension(FileName);
	options.setDirectUrl(directUrl);
	options.setThumbUrl(thumbUrl);
	options.setViewUrl(viewUrl);
	
	return 1;
}

function GetFolderAccessTypeList()
{
	local a=["Приватный", "Для всех"];
	return a;
}

function GetServerParamList()
{
	return 
	{
		token = "token",
		tokenType = "TokenType",
		OAuthLogin = "OAuthLogin",
		PrevLogin = "PrevLogin"
	}
	return a;
}