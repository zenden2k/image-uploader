/*
	Image Uploader scriptable add-on
	Author: Alexamder Mikhnevich aka @arhangelsoft at github.com
	
	Requsits:
		imgur.com user login: g1680003
		imgur.com user mail: g1680003@mvrht.com
		imgur.com user password: g1680003mvrhtcom
	
*/

function  UploadFile(FileName, options)
{	
    nm.setUrl("https://api.imgur.com/3/image");
	nm.addQueryHeader("Authorization","Client-ID 41b31959cfe4d8d");
    nm.addQueryParamFile("image", FileName, ExtractFileName(FileName),"");
    nm.doUploadMultipartData();
	if (nm.responseCode() == 200) {
		local retdoc = nm.responseBody();
		local json = ParseJSON(retdoc);
		if (json != null) {
			if (json.success == true) {
				local directUrl = json.data.link;
				options.setDirectUrl(directUrl);
				return 1;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
	} else {
		return 0;
	}
}