
function UploadFile(FileName, options) {
    local name = ExtractFileName(FileName);
    local mime = GetFileMimeType(name);
    local tm = time().tostring();
    local hash = md5("5CE3DF4D45AC*" + tm);
    local imageInfo = GetImageInfo(FileName);
    if (imageInfo.Width == 0 || imageInfo.Height == 0) {
        imageInfo.Width = 640;
        imageInfo.Height = 480;
    }
    nm.setUrl("http://upload.prntscr.com/upload/" + tm + "/" + hash + "/" );
    nm.addQueryParamFile("image", FileName, name, mime);
    nm.doUploadMultipartData();
    
    
    if (nm.responseCode() == 200) {
        local xml = SimpleXml();
        if(xml.LoadFromString(nm.responseBody())) {
            local root = xml.GetRoot("response", false);
            local statusNode = root.GetChild("status", false);
            if (statusNode.Text() != "success"){
                return 0;
            }
            local directUrl = root.GetChild("url", false).Text();
            local thumb = root.GetChild("thumb", false).Text();
            
            if (directUrl == "" || thumb == "") {
                return 0;
            }
            
            nm.setUrl("https://api.prntscr.com/v1/");
            local request = { jsonrpc = "2.0",
                method = "save",
                id   = 1,
                params = {
                    img_url = directUrl,
                    thumb_url = thumb,
                    delete_hash = "",
                    app_id = "{236D39BE-786B-4FEB-919D-F0CF797C153E}",
                    width = imageInfo.Width,
                    height = imageInfo.Height,
                    dpr  = 1.0
                }
            };
            nm.doPost(ToJSON(request));
            if (nm.responseCode() == 200) {
                local t = ParseJSON(nm.responseBody());
                if ("result" in t && t.result.success) {
                    options.setViewUrl(t.result.url);
                    options.setThumbUrl(thumb);
                    options.setDirectUrl(directUrl);
                    return 1;   
                }
            }
        }
    }        
    
    return 0;
}