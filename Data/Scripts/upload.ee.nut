baseUrl <- "https://www.upload.ee";

function UploadFile(FileName, options) {
    const finishUrl = "/?page=finished&upload_id=";
    nm.setReferer(baseUrl);
    nm.doGet(baseUrl + "/ubr_link_upload.php?rnd_id=" + time());

    if (nm.responseCode() == 200) {
        local reg = CRegExp("startUpload\\(\"(.+?)\"", "mi");
                           
        if ( reg.match(nm.responseBody()) ) {
            local uploadId = reg.getMatch(1);
            
            nm.setReferer(baseUrl);
            nm.setUrl(baseUrl + "/cgi-bin/ubr_upload.pl?X-Progress-ID=" + nm.urlEncode(uploadId)+ "&upload_id=" + nm.urlEncode(uploadId));

            nm.addQueryParamFile("upfile_0", FileName, ExtractFileName(FileName), GetFileMimeType(FileName));
            nm.addQueryParam("link", "");
            nm.addQueryParam("email", "");
            nm.addQueryParam("category", "cat_file");
            nm.addQueryParam("big_resize", "none");
            nm.addQueryParam("small_resize", "120x90");
            
            nm.doUploadMultipartData();

            if (nm.responseCode() == 200) {
                local data = nm.responseBody();

                if (data.find(finishUrl, 0) == null) {
                    WriteLog("error", "[upload.ee] Upload failed");
                    return 0;
                }

                nm.doGet(baseUrl + finishUrl +  nm.urlEncode(uploadId));

                if (nm.responseCode() == 200) {
                    local doc = Document(nm.responseBody());
                    local downloadUrl = doc.find("input#file_src").at(0).attr("value");
                    local imageUrl = doc.find("input#image_src").at(0).attr("value");
                    local thumbUrl = doc.find("input#thumb_src").at(0).attr("value");

                    if (downloadUrl == "") {
                        WriteLog("error", "[upload.ee] Unable to find File URL on the page");
                        return 0;
                    }
                    options.setViewUrl(downloadUrl);
                    options.setDirectUrl(imageUrl);
                    options.setThumbUrl(thumbUrl);
                    local reg2 = CRegExp("href=\"(.+\\?killcode=\\w+)\"", "i");
                    if (reg2.match(nm.responseBody())) {
                        options.setDeleteUrl(reg2.getMatch(1))
                    }
                    
                    return 1;
                } 
            }
        } else {
            WriteLog("error", "[upload.ee] Unable to obtain Upload ID");
        }
    }

    return 0;
}