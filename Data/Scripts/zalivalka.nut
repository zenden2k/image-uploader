regMatchOffset <- 0;
try {
	local ver = GetAppVersion();
	if ( ver.Build > 4422 ) {
		regMatchOffset = 1;
	}
} catch ( ex ) {
}
uploadUrl <- "";
function GetUploadUrl() {
	try {
		return Sync.getValue("uploadUrl");
	} catch ( ex ) {
		
	}
	return uploadUrl;
}

function SetUploadUrl(url) {
	try {
		Sync.setValue("uploadUrl", url);
		return;
	} catch ( ex ) {
		
	}
	uploadUrl = url;
}

function  UploadFile(FileName, options)
{		
	local url = GetUploadUrl();
	if ( url == "" ) {
		nm.doGet("http://zalivalka.ru");
		if ( nm.responseCode() != 200 ) {
			return 0;
		}
		local reg = CRegExp("action=\"(http:\\/\\/.+?\\.zalivalka\\.ru\\/upload\\.php)","i");
		if ( !reg.match(nm.responseBody()) ) {
			return 0;
		}
		url = reg.getMatch(regMatchOffset + 0);
		SetUploadUrl(url);
	}
	
	nm.setUrl(url);
	nm.addQueryParamFile("userfile",FileName, ExtractFileName(FileName),"");
	nm.addQueryParam("fileName[]", ExtractFileName(FileName));
	nm.addQueryParam("ajax_upload", "1");
	nm.doUploadMultipartData();
	if ( nm.responseCode() != 200 )  {
		return 0;
	}
 	local data = nm.responseBody();
	local t = ParseJSON(data);
	if ( t != null && t.result == 1 ) {
		options.setDirectUrl(t.links.direct);
		options.setViewUrl("http://zalivalka.ru/" + t.fhf_id);
		try {
			options.setThumbUrl(t.links.thumb);
		} catch ( ex ) {
		}
		try {
			options.setEditUrl(t.links.actions);
		} catch ( ex ) {
		}

		return 1;
	}
		
	return 0;
}