/*
	Image Uploader scriptable add-on
	Author: Alexamder Mikhnevich aka @arhangelsoft at github.com
	
	Requsits:
		imgur.com user login: j3uoxsn0spd
		imgur.com user mail: j3uoxsn0.spd@20mail.eu
		imgur.com user password: j3uoxsn0spd20maileu
		
		client id: 74bbb480ed31539
		secret: 64e240cbe6e74744aad769b85636d43239ff3a28
	
*/

function  UploadFile(FileName, options)
{	
    nm.setUrl("https://api.imgur.com/3/image");
	nm.addQueryHeader("Authorization","Client-ID 74bbb480ed31539");
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