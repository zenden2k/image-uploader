trackKey <- "";
ukey <- "";
user <- "";
uploadkey <- "";
MFULConfig <- "";


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

function Login() {
	local email = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	
	nm.doGet( "http://www.mediafire.com/myaccount.php");
	nm.doGet( "http://www.mediafire.com/" );
	
	nm.setUrl( "http://www.mediafire.com/dynamic/login.php");
	nm.addQueryParam( "login_email", email );
	nm.addQueryParam( "login_pass", pass );
	nm.addQueryParam( "login_remember", "on" );
	nm.addQueryParam( "submit_login", "Login to MediaFire" );
	nm.doPost("");
	local data =  nm.responseBody();
	return 1;
}

function  UploadFile(FileName, options) {	
	local email = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	if ( email != "" && pass !="" && uploadkey == "" ) {
		Login();
	
	}
	
	if ( uploadkey == "" ) {
		nm.doGet( "http://www.mediafire.com/myaccount.php");
		nm.doGet("http://www.mediafire.com/basicapi/uploaderconfiguration.php");
		local data = nm.responseBody();
		trackKey = regex_simple(data, "<trackkey>(.+)</trackkey>", 0);
		ukey = regex_simple(data, "<ukey>(.+)</ukey>", 0);
		user = regex_simple(data, "<user>(.+)</user>", 0);
		uploadkey = regex_simple(data, "<folderkey>(.+)</folderkey>", 0);
		MFULConfig = regex_simple(data, "<MFULConfig>(.+)</MFULConfig>", 0);
		//DebugMessage( "trackkey=" + trackKey + "\r\nukey=" + ukey + "\r\nuser=" + user + "\r\nuploadkey" + uploadkey , true );
	}
	
	
	local uploadUrl = "http://www.mediafire.com/douploadtoapi/?track=" + trackKey + 
	    "&ukey=" + ukey + "&user=" + user + 
		 "&uploadkey=" + uploadkey + "&upload=0";
	nm.setUrl( uploadUrl );
	nm.addQueryParam( "Filename", ExtractFileName(FileName) );
	nm.addQueryParam( "Upload", "Submit Query" );
	nm.addQueryParamFile("Filedata",FileName, ExtractFileName(FileName),"");
	nm.doUploadMultipartData();
	
 	local data = nm.responseBody();
	local key = regex_simple(data, "<key>(.+)</key>", 0);
	
	local pollingUrl =  "http://www.mediafire.com/basicapi/pollupload.php?key=" + key + "&MFULConfig=" +  MFULConfig;
	local status = "";
	local downloadKey = "";
	
	while ( ( status != "99" ) && downloadKey == "" ) {
		nm.doGet( pollingUrl );
		data = nm.responseBody();
		status = regex_simple(data, "<status>(.+)</status>", 0);
		downloadKey = regex_simple(data, "<quickkey>(.+)</quickkey>", 0);
		local description = regex_simple(data, "<description>(.+)</description>", 0);
		
			if ( status == "5" ) {
			downloadKey = regex_simple(data, "<quickkey>(.+)</quickkey>", 0);
		}
		
		if ( description == "No more requests for this key" ) {
			break;
		}
	}
	
	if ( downloadKey != "" ) {
		options.setViewUrl( "http://www.mediafire.com/?" + downloadKey );
		return 1;
	}
	return 0;
}