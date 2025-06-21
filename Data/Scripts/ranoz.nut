function UploadFile(FileName, options) {
    local fileSize = GetFileSize(FileName);
	local task = options.getTask().getFileTask();
    local newFilename = task.getDisplayName();
    
    // Step 1: Get pre-signed upload URL
    nm.setUrl("https://ranoz.gg/api/v1/files/upload_url");
    nm.addQueryHeader("Content-Type", "application/json");
    
    // Prepare JSON data
    local requestData = {
        filename = newFilename,
        size = fileSize
    };
    
    local jsonData = ToJSON(requestData);
    
    nm.doPost(jsonData);
    
    if (nm.responseCode() != 200) {
        WriteLog("error", "[ranoz.gg] Failed to get upload URL. HTTP code: " + nm.responseCode());
        return 0;
    }
    
    local response = nm.responseBody();
    
    // Parse response to get upload URL and file data
    local responseData = ParseJSON(response);
    if (responseData == null || !("data" in responseData)) {
        WriteLog("error", "[ranoz.gg] Failed to parse response data");
        return 0;
    }
    
    local data = responseData.data;
    if (!("upload_url" in data)) {
        WriteLog("error", "[ranoz.gg] No upload_url in response");
        return 0;
    }
    
    local uploadUrl = data.upload_url.tostring();
    
    // Extract file information for later use
    local fileUrl = ("url" in data) ? data.url.tostring() : "";
    
    // Step 2: Upload file to pre-signed URL using PUT
    nm.setUrl(uploadUrl);
    nm.setMethod("PUT");
    nm.addQueryHeader("Content-Type", "application/octet-stream");
    
    // Upload file
    nm.doUpload(FileName, "");
    
    local uploadResponseCode = nm.responseCode();
    
    // For PUT upload, success codes can be 200, 201, or 204
    if (uploadResponseCode != 200 && uploadResponseCode != 201 && uploadResponseCode != 204) {
        WriteLog("error", "[ranoz.gg] Failed to upload file. HTTP code: " + uploadResponseCode);
        return 0;
    }
    
    if (fileUrl != "") {
        options.setViewUrl(fileUrl);
    }
    
    return 1;
}