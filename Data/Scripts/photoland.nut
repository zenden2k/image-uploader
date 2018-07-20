// Chevereto uploader
function UploadFile(FileName, options) {
	local name = ExtractFileName(FileName);
	local mime = GetFileMimeType(name);
	nm.setUrl("https://photoland.io/api/1/upload");
    nm.addQueryParam("key", "81265619f65dc2c0c09bde97ca9a6906");
    nm.addQueryParam("format", "json");
    nm.addQueryParamFile("source", FileName, name, mime);
	nm.doUploadMultipartData();
    
	local sJSON = nm.responseBody();
	local t = ParseJSON(sJSON);
	if (t != null) {
        if (t.status_code == 200){
            options.setViewUrl(t.image.url_viewer );
            options.setThumbUrl(t.image.thumb.url);
            options.setDirectUrl(t.image.url);
            return 1;
        } else {
            if(t.error!=null) {
                WriteLog("error", "photoland.io: " + t.error.message);
            }
        }
	} 
    
    return 0;
}