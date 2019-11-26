
function UploadFile(FileName, options) {
    local apiKey = ServerParams.getParam("apiKey");
    local login = ServerParams.getParam("Login");
    if ( login != "" && apiKey == "") {
        WriteLog("error", "myfile.is: You must set API Key in server settings for uploading into account.");
        return 0;
    }
    local name = ExtractFileName(FileName);
    local mime = GetFileMimeType(name);
    local url = "https://api.myfile.is/upload";
    if (apiKey != "") {
        url += "?token=" + apiKey;
    }
    nm.setUrl(url);
    nm.addQueryParamFile("file", FileName, name, mime);
    nm.doUploadMultipartData();
    if (nm.responseCode() == 200) {
        local sJSON = nm.responseBody();
        local t = ParseJSON(sJSON);
        if (t != null) {
            if (t.status) {
                //options.setDirectUrl(t.data.file.url.full);
                options.setViewUrl(t.data.file.url.short);
                return 1;
            } else {
                local msg = "";
                if ( "error" in t) {
                    msg = t.error.message;
                }
                WriteLog("error", "myfile.is: Upload failed. Error: " + msg);
            }
            
        } else {
            WriteLog("error", "myfile.is: Failed to parse server's answer aj json.");
        }

    } else {
        WriteLog("error", "myfile.is: Failed to upload. Server status code: " + nm.responseCode());
    }
    return 0;
}

function GetServerParamList()
{	
	return {
        apiKey = "API Key",
	};
}