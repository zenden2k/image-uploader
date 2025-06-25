const BASE_URL = "https://api.filemail.com";
const CHUNK_SIZE = 5242880; // 5MB chunks

function _InitRequest() {
    nm.addQueryHeader("x-api-version", "2.0");
    nm.addQueryHeader("source", "desktop");
}

function _PrintError(t, txt) {
    local errorMessage = "[Filemail.com] " + txt + " Response code: " + nm.responseCode();
    if (t != null && "errormessage" in t) {
        errorMessage += "\n" + t.errormessage;
    }
    WriteLog("error", errorMessage);
}

// Upload file to Filemail.com
// @param string pathToFile 
// @param UploadParams options
// @return int - success(1), failure(0)
function UploadFile(pathToFile, options) {
    local task = options.getTask().getFileTask();
    local loginToken = ServerParams.getParam("loginToken");
    local email = ServerParams.getParam("email");
    local password = ServerParams.getParam("password");
    local fileSize = GetFileSize(pathToFile);
    // Initialize transfer
    local transferData = InitializeTransfer(loginToken, task.getDisplayName(), fileSize);
    if (transferData == null) {
        WriteLog("error", "[Filemail.com] Failed to initialize transfer");
        return ResultCode.Failure;
    }
    
    // Upload file
    if (!UploadFileToTransfer(pathToFile, transferData, task.getDisplayName(), fileSize)) {
        WriteLog("error", "[Filemail.com] Failed to upload file");
        return ResultCode.Failure;
    }
    
    // Complete transfer
    local downloadUrl = CompleteTransfer(transferData);
    if (downloadUrl == "") {
        WriteLog("error", "[Filemail.com] Failed to complete transfer");
        return ResultCode.Failure;
    }
    
    // Set URLs
    options.setViewUrl(downloadUrl);
    
    return ResultCode.Success;
}

// Authenticate user and get login token
function Authenticate() {
    local email = ServerParams.getParam("Login");
    local password = ServerParams.getParam("Password");
    
    if (email == "" || password == "") {
        return ResultCode.Failure;
    }

    local req = {
        email = email,
        password =  password
        source = "Desktop"
    };
    
    nm.setUrl(BASE_URL + "/auth/login");
    _InitRequest();
    nm.addQueryHeader("Content-Type", "application/json");
    nm.doPost(ToJSON(req));
    
    if (nm.responseCode() != 200) {
        WriteLog("error", "[Filemail.com] Authentication request failed. Response code: " + nm.responseCode());
        return ResultCode.Failure;
    }
    
    local response = ParseJSON(nm.responseBody());
    if (response == null) {
        WriteLog("error", "[Filemail.com] Failed to parse authentication response");
        return ResultCode.Failure;
    }
    
    if ("responsestatus" in response && response.responsestatus == "OK" && "logintoken" in response.data) {
        ServerParams.setParam("loginToken", response.data.logintoken);
        ServerParams.setParam("logintokenExpireDate", response.data.logintokenExpireDate);
        ServerParams.setParam("refreshtoken", response.data.refreshtoken);
        return ResultCode.Success;
    } else {
        WriteLog("error", "[Filemail.com] Authentication failed: " + nm.responseBody());
    }
    return ResultCode.Failure;
}

// Initialize transfer and get transfer data
// @param string loginToken
// @param string fileName
// @return table - transfer data or null on failure
function InitializeTransfer(loginToken, fileName, fileSize) {
    local url = BASE_URL + "/transfer/initialize";
    
    nm.setUrl(url);
    _InitRequest();

    local req =  {
        token = loginToken//,
        //transfersize = fileSize
    };
    // Add optional parameters
    
    local subject = ServerParams.getParam("subject");

    if (subject == "") {
        subject = "File: " + fileName;
    }
    
    req.subject <- subject;

    local message = ServerParams.getParam("message");
    if (message != "") {
        req.message <- message;
    }
    
    local days = ServerParams.getParam("days");
    if (days != "") {
        req.days <- days;
    }
    
    local notify = ServerParams.getParam("notify");
    if (notify != "") {
        req.notify <- notify;
    }
    
    local confirmation = ServerParams.getParam("confirmation");
    if (confirmation != "") {
        req.confirmation <- confirmation;
    }
    
    nm.addQueryHeader("Content-Type", "application/json");
    nm.doPost(ToJSON(req));
    local response = ParseJSON(nm.responseBody());
    
    if (nm.responseCode() != 200) {
        _PrintError(response, "Transfer initialization failed.");
        return null;
    }
    
    if (response == null) {
        WriteLog("error", "[Filemail.com] Failed to parse transfer initialization response");
        return null;
    }
    
    if ("responsestatus" in response && response.responsestatus == "OK") {
        return response.data;
    } else {
        _PrintError(response, "Transfer initialization failed.");
        return null;
    }
}

// Upload file to initialized transfer
// @param string pathToFile
// @param table transferData
// @param string fileName
// @return bool - success(true), failure(false)
function UploadFileToTransfer(pathToFile, transferData, fileName, fileSize) {
    if (!("transferurl" in transferData) || !("transferid" in transferData) || !("transferkey" in transferData)) {
        WriteLog("error", "[Filemail.com] Invalid transfer data");
        return false;
    }
    
    local chunkSize = CHUNK_SIZE;
    local chunks = fileSize > 0 ? ((fileSize + chunkSize - 1) / chunkSize).tointeger() : 1;
    
    // For files smaller than chunk size, upload without chunking
    if (fileSize <= chunkSize) {
        return UploadFileChunk(pathToFile, transferData, fileName, 0, fileSize, 0, 1);
    }
    

    for (local chunkIndex = 0; chunkIndex < chunks; chunkIndex++) {
        local bytesToRead = (chunkIndex == chunks - 1) ? (fileSize - chunkIndex * chunkSize) : chunkSize;
        
        if (!UploadFileChunk(pathToFile, transferData, fileName, chunkIndex * chunkSize, bytesToRead, chunkIndex, chunks)) {
            return false;
        }
    }
    
    return true;
}

// Upload a single chunk of the file
// @param string pathToFile
// @param table transferData
// @param string fileName
// @param int offset
// @param int size
// @param int chunkIndex
// @param int totalChunks
// @return bool - success(true), failure(false)
function UploadFileChunk(pathToFile, transferData, fileName, offset, size, chunkIndex, totalChunks) {
    local url = transferData.transferurl + "?transferid=" + nm.urlEncode(transferData.transferid) 
                                        + "&transferkey=" + nm.urlEncode(transferData.transferkey)
                                        + "&thefilename=" + nm.urlEncode(fileName)
                                        +"&md5=" + Md5(GetFileContentsEx(pathToFile, offset, size, true));
    
    if (totalChunks > 1) {
        url += "&chunksize=" + CHUNK_SIZE + "&chunks=" +totalChunks.tostring() + "&chunk=" + chunkIndex.tostring();
    }

    nm.setUrl(url);
    _InitRequest();

    // Upload file data
    nm.addQueryHeader("Content-Type", "application/octet-stream");
    
    nm.setChunkOffset(offset);
    nm.setChunkSize(size);

    nm.doUpload(pathToFile, "");
    
    if (nm.responseCode() == 406 || nm.responseCode() == 449) {
        WriteLog("info", "[Filemail.com] Retrying chunk upload (HTTP " + nm.responseCode() + ")");
        // Retry the chunk
        nm.doUpload(pathToFile, "");
    }
    local t = ParseJSON(nm.responseBody());

    if (nm.responseCode() != 200) {
        WriteLog("error", "[Filemail.com] Chunk upload failed. Response code: " + nm.responseCode());
        return false;
    }
    
    return true;
}

// Complete the transfer and get download URL
// @param table transferData
// @return string - download URL or empty string on failure
function CompleteTransfer(transferData) {
    local url = BASE_URL + "/transfer/complete";
    
    nm.setUrl(url);
    nm.setMethod("PUT");
    _InitRequest();

    local req = {
        transferid = transferData.transferid,
        transferkey = transferData.transferkey
    }
    
    /*local loginToken = ServerParams.getParam("loginToken");
    if (loginToken != "") {
        nm.addQueryParam("logintoken", loginToken);
    }*/
    
    nm.addQueryHeader("Content-Type", "application/json");
    
    nm.doUpload("", ToJSON(req));
    
    local response = ParseJSON(nm.responseBody());

    if (nm.responseCode() != 200) {
         _PrintError(response, "Transfer completion failed.");
        return "";
    }
    
    if (response == null) {
        _PrintError(response, "Failed to parse transfer completion response.");
        return "";
    }
    
    if ("responsestatus" in response && response.responsestatus == "OK" && "downloadurl" in response.data) {
        return response.data.downloadurl;
    } else {
        _PrintError(response, "Transfer completion failed.");
        return "";
    }
}

// Optional function: Get list of server parameters
function GetServerParamList() {
    return {
        subject = "Email Subject",
        message = "Message",
        days = "Days to keep file (default: account setting)",
        notify = {
            title = "Email notification on download",
            type = "boolean"
        },
        confirmation = {
            title = "Email confirmation when sent",
            type = "boolean"
        }
    }
}