function UploadFile(FileName, options) {	
    nm.setUrl("https://pastenow.ru/");
    nm.addQueryParamFile("file", FileName, ExtractFileName(FileName), "");
    nm.doUploadMultipartData();
    
    if ( nm.responseCode() == 200) {
        local doc = Document(nm.responseBody());
        local viewUrl = doc.find("#paste-url").attr("value");
        options.setViewUrl(viewUrl);
        local directUrl = doc.find("#paste-img-url").attr("value");
        options.setDirectUrl(directUrl);
        if (directUrl != "" || viewUrl != "") {
            return 1; 
        }
    }

    return 0;
}