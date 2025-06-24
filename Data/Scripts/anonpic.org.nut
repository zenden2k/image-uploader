const BASE_URL = "https://anonpic.org";

// This is the function that performs the upload of the file
// @param string pathToFile 
// @param UploadParams options
// @return int - success(1), failure(0)
function UploadFile(pathToFile, options) {
    local task = options.getTask().getFileTask();
    
    // Set up the upload URL
    nm.setUrl(BASE_URL + "/upload.php");
    
    // Add form parameters
    nm.addQueryParam("MAX_FILE_SIZE", "10485760"); // 10MB limit
    nm.addQueryParam("imgUrl", ""); // Empty imgUrl field
    nm.addQueryParam("fileName[]", "C:\\fakepath\\" + task.getDisplayName());
    
    // Add the file
    nm.addQueryParamFile("file[]", pathToFile, task.getDisplayName(), GetFileMimeType(pathToFile));
    
    // Perform multipart upload
    nm.doUploadMultipartData();
    
    if (nm.responseCode() != 200) {
        WriteLog("error", "[anonpic.org] Upload failed. Response code: " + nm.responseCode());
        return ResultCode.Failure;
    }
    
    // Parse HTML response with Gumbo-query
    local doc = Document(nm.responseBody());
    
    // Extract direct link from "Direct Link" input field
    local directUrl = "";
    local directLinkInput = doc.find("input#codedirect");
    if (directLinkInput.length() > 0) {
        directUrl = directLinkInput.attr("value");
    }
    
    // Extract BBCode link for view URL
    local viewUrl = "";
    /*local bbCodeInput = doc.find("input#codebb");
    if (bbCodeInput.length() > 0) {
        local bbCodeValue = bbCodeInput.attr("value");
        // Extract URL from BBCode using regex
        local bbCodeReg = CRegExp("\\[IMG\\]([^\\]]*)\\[/IMG\\]", "mi");
        if (bbCodeReg.match(bbCodeValue)) {
            viewUrl = bbCodeReg.getMatch(1);
        }
    }*/
    
    // Extract delete URL from HTML comment using regex
    local deleteUrl = "";
    local responseBody = nm.responseBody();
    local deleteReg = CRegExp("value=\"([^\"]*\\?d=[^\"]*)\"|value='([^']*\\?d=[^']*)'", "mi");
    
    if (deleteReg.match(responseBody)) {
        deleteUrl = deleteReg.getMatch(1);
        if (deleteUrl == "") {
            deleteUrl = deleteReg.getMatch(2);
        }
    }
    
    // Check if we got the required URLs
    if (directUrl == "") {
        WriteLog("error", "[anonpic.org] Upload failed. Cannot obtain the direct URL!");
        return ResultCode.Failure;
    }
    
    // Set the URLs in options
    options.setDirectUrl(directUrl);
    options.setViewUrl(viewUrl);
    options.setThumbUrl(directUrl); // Use direct URL as thumbnail URL since service doesn't provide separate thumbnail
    
    // Store delete URL in edit URL field if available
    if (deleteUrl != "") {
        options.setEditUrl(deleteUrl);
    }
    
    WriteLog("info", "[anonpic.org] Upload successful. Direct URL: " + directUrl);
    
    return ResultCode.Success;
}