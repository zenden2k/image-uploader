function  UploadFile(FileName, options)
{	
	nm.setUrl("http://savephoto.ru/Apix/Handler.ashx");
	nm.addQueryParam("wuid", "730C-491A-6BEF-C5B4-1837-ED0E-47F1-F73D");
    nm.addQueryParam("file", Base64Encode(GetFileContents(FileName)));

	nm.doPost("");
    
    if ( nm.responseCode() == 200) {
        local data = nm.responseBody();
        options.setViewUrl(data);
        return 1; 
    }

	return 0;
}