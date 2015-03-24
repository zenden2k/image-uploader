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
	nm.setUrl("http://www.pixhost.org/api");
	nm.addQueryParam("content_type", "0");
	nm.addQueryParamFile("img", FileName, name, mime);
	nm.doUploadMultipartData();
	if (nm.responseCode() == 200) {
		local sJSON = nm.responseBody();
		local tJSON = ParseJSON(sJSON);
		if (tJSON != null) {
			local surl = "http://www.pixhost.org/show/";
			local sdirect_url = "";
			surl += tJSON.dir + "/" + tJSON.file;

			nm.doGet(surl);
			if (nm.responseCode() == 200) {
				sdirect_url = regex_simple(nm.responseBody(), "<img id=\"show_image\" src=\"(.+)\" onClick=\"", 0);
			}

			options.setViewUrl(surl);
			options.setDirectUrl(sdirect_url);
			return 1;
		} else
			return 0;

	} else
		return 0;

}