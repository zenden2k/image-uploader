function regex_simple(data, regStr, start) {
	local ex = regexp(regStr);
	local res = ex.capture(data, start);
	local resultStr = "";
	if (res != null) {
		resultStr = data.slice(res[1].begin, res[1].end);
	}
	return resultStr;
}

function UploadFile(FileName, options) {
	local name = ExtractFileName(FileName);
	local mime = GetFileMimeType(name);
	nm.setUrl("http://pimpandhost.com/upload/post");
	nm.addQueryParam("name", name);
	nm.addQueryParam("return", "json");
	nm.addQueryParam("albumId", "0");
	nm.addQueryParamFile("Filedata", FileName, name, mime);
	nm.doUploadMultipartData();
	if (nm.responseCode() == 200) {
		local sJSON = nm.responseBody();
		local t = ParseJSON(sJSON);
		if (t != null && !t.error) {
			options.setViewUrl(t.url);
            
            local ver = GetAppVersion();
            if (ver && ver.Build >= 4451) {
                nm.doGet(t.url);
                if (nm.responseCode() == 200) {
                    local doc = Document(nm.responseBody());
                    options.setDirectUrl(doc.find("#image").at(0).attr("src"));
                }
            }
			
			options.setThumbUrl(t.thumb);
			return 1;
		} else {
			return 0;
            }

	} else {
		return 0;
    }
}