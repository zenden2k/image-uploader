function UploadFile(FileName, options) {
	local name = ExtractFileName(FileName);
	local mimeType = GetFileMimeType(FileName);
	nm.setUrl("http://img.5cm.ru/api/upload?v=2");
	nm.addQueryHeader("User-Agent", "Windows/2.0.0");
	nm.addQueryHeader("Content-Type", mimeType);
	
	nm.setMethod("POST");
	nm.doUpload(FileName, "");
	if (nm.responseCode() == 200) {
		local xml = SimpleXml();
		xml.LoadFromString(nm.responseBody());
		local uploadNode = xml.GetRoot("upload", false);
		if ( !uploadNode.IsNull() ) {
			local count = uploadNode.GetChildCount();
			local directUrl = null;
			for ( local i = 0; i < count ; i++ ) {
				local linkNode = uploadNode.GetChildByIndex(i);
				if ( i == 0 ) {
					directUrl = linkNode.Text();
					
				} else if ( i == 9 ) {
					options.setViewUrl(linkNode.Text());
				}else if ( i == 6 ) {
					options.setThumbUrl(linkNode.Text());
				}
			}
			if ( directUrl ) {
				options.setDirectUrl(directUrl);
				return 1;
			}
		
		} 
	} 
	return 0;

}