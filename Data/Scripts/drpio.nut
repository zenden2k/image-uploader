/*
	server: drp.io
	type: any file
	keep files: up to 1 week
*/

function UploadFile(FileName, options) {
	nm.setUrl("http://drp.io:8080/api/upload");
	nm.addQueryParamFile("file0", FileName, ExtractFileName(FileName), "");
	nm.addQueryParam("selfdestructIn", "1w");
	nm.addQueryParam("emails", "[]");
	nm.doUploadMultipartData();
	if (nm.responseCode() == 200) {
		local directUrl = "";
		local viewUrl = "";
		local data = nm.responseBody();
		local JO = ParseJSON(data);
		directUrl = JO.file.url;
		viewUrl = "http://drp.io/" + JO.file._id;
		options.setDirectUrl(directUrl);
		options.setViewUrl(viewUrl);
		return 1;
	} else
		return -1;
}
