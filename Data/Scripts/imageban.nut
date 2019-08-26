MyClientId <- "tB3J94mijYhW5Up5fm2c";

function getThumbnailWidth() {
	local result = "180";
	try{
		result = options.getParam("THUMBWIDTH");
	}
	catch(ex)
	{
	}
	return result;
}

function anonymousUpload(FileName, options) {
	nm.setUrl("https://imageban.ru/up");
	nm.addQueryHeader("User-Agent", "Shockwave Flash");
	nm.addQueryParam("Filename", ExtractFileName(FileName));
	nm.addQueryParam("albmenu", "0");
	nm.addQueryParam("grad", "0");
	nm.addQueryParam("rsize", "0");
	nm.addQueryParam("inf", "1");
	nm.addQueryParam("prew", getThumbnailWidth());
	nm.addQueryParam("ptext", "");
	nm.addQueryParam("rand", format("%d",random()%22222));
	nm.addQueryParam("ttl", "0");
	nm.addQueryParamFile("Filedata",FileName, ExtractFileName(FileName),"");
	nm.addQueryParam("Upload", "Submit Query");
	
	nm.doUploadMultipartData();

    if (nm.responseCode() == 200 ) {
        local t = ParseJSON(nm.responseBody());
        if ("files" in t && t.files.len()) {
            local file = t.files[0];
            local directUrl = file.link;
            if ("thumbs" in file) {
                options.setThumbUrl(file.thumbs);
            }
            if ("piclink" in file) {
                options.setViewUrl(file.piclink);
            }
            if ("delete" in file) {
                options.setDeleteUrl(file.rawget("delete"));
            }
            
            options.setDirectUrl(directUrl);
          
            if ( directUrl != "") {
                return 1;
            }
        }
    } else {
        WriteLog("error", "")
    }
	
	return 0;
}

function  UploadFile(FileName, options)
{	
    local login = ServerParams.getParam("Login");
	
    if ( login == "" ) {
        return anonymousUpload(FileName, options);
    }
    
    local secretKey = ServerParams.getParam("SecretKey");
    local clientId = ServerParams.getParam("ClientId");
    if (clientId == "") {
        clientId = MyClientId;
    }
    if (secretKey == "" ){
        WriteLog("error", "imageban.ru: SecretKey parameter cannot be empty. \r\nYou must set SecretKey in server settings.");
        return 0;
    }
	
	nm.setUrl("https://api.imageban.ru/v1");
    nm.addQueryHeader("Authorization", "TOKEN " + clientId);
	nm.addQueryParamFile("image",FileName, ExtractFileName(FileName),"");
	nm.addQueryParam("secret_key", secretKey);
	nm.doUploadMultipartData();
    
    if (nm.responseCode() == 200) {
        local data = nm.responseBody();
        local t = ParseJSON(data);
        if ("success" in t && t.success) {
            local viewUrl  	= t.data.short_link;
            local directUrl  	= t.data.link;
            
            options.setDirectUrl(directUrl);
            options.setViewUrl(viewUrl);
            return 1; // Success
        } else {
            if ("error" in t) {
                WriteLog("error", "imageban.ru: " + t.error.message);
            } else {
                 WriteLog("error", "imageban.ru: Unknown error");
            }
        } 
    } else {
         WriteLog("error", "imageban.ru: Upload failed. Response code: " + nm.responseCode());
    }        
	
	return 0;
}

function GetServerParamList()
{
    return { 
		ClientId = "ClientId",
        SecretKey = "SecretKey"
	};
}