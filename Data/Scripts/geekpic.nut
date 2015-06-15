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
	//nm.setUrl("http://geekpic.net/ajax.php");
	nm.setUrl("http://geekpic.net/client.php");
    nm.addQueryHeader("MIME-Version", "1.0");
    nm.addQueryHeader("Accept-Language", "ru-RU,en,*");
    nm.setUserAgent("Mozilla/5.0 (Windows NT 6.3) Qt/4.8.6 Screenpic/0.14.3");
    nm.addQueryParam("image", Base64Encode(GetFileContents(FileName)));
    nm.addQueryParam("name", name);
    nm.addQueryParam("description", name);
    
	//nm.addQueryParamFile("file", FileName, name, mime);
	nm.doUploadMultipartData();
	if (nm.responseCode() == 200) {
		local sJSON = nm.responseBody();
		local t = ParseJSON(sJSON);
		if (t != null && t.success) {
			options.setViewUrl(t.preview);
            options.setThumbUrl(t.thumbs.small);
			options.setDirectUrl(t.link);
			return 1;
		} else
			return 0;

	} else
		return 0;

}