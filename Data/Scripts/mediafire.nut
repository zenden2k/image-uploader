function min(a,b) {
    return a < b ? a : b;
}

function _GetSecurityToken() {
    nm.doGet("https://www.mediafire.com/login/");
    if (nm.responseCode() == 200) {
        local doc = Document(nm.responseBody());
        local inputSecurity = doc.find("input[name=security]");
        if (inputSecurity.length()) {
            local token = inputSecurity.attr("value");
            if (token != "") {
                return token;
            } else {
                WriteLog("error", "[mediafire.com] Cannot grab security token!");
            }
        } else {
            WriteLog("error", "[mediafire.com] Cannot find security input");
        }
    } else {
        WriteLog("error", "[mediafire.com] Cannot grab security token!");
    }
}

function Authenticate() {
    local login = ServerParams.getParam("Login");
    local password = ServerParams.getParam("Password");
    local securityToken = _GetSecurityToken();

    nm.setUrl("https://www.mediafire.com/dynamic/client_login/mediafire.php");
    nm.addQueryParam("security", securityToken);
    nm.addQueryParam("login_email", login);
    nm.addQueryParam("login_pass", password);
    nm.addQueryParam("login_remember", "on");
    nm.setReferer("https://www.mediafire.com/login/");

    nm.doPost("");

    if (nm.responseCode() == 200) {
        nm.setUrl("https://www.mediafire.com/application/get_session_token.php");
        nm.doPost("session_token=&response_format=json");
        nm.setReferer("https://www.mediafire.com/login/");
        local t = ParseJSON(nm.responseBody());
        if ("response" in t) {
            local sessionToken = t.response.session_token;
            if (sessionToken == "") {
                WriteLog("error", "[mediafire.com] Failed to authenticate: empty session token");
                return 0;
            } 
            Sync.setValue("sessionToken", sessionToken);
            return 1;
        } else {
            WriteLog("error", "[mediafire.com] Failed to authenticate, response code=" + nm.responseCode());
        }
        
    } else {
        local t = ParseJSON(nm.responseBody());
        local errorStr = "";
        if ("errorMessage" in t) {
            errorStr = t.errorMessage;
        }
        WriteLog("error", "[mediafire.com] Failed to authenticate: " + errorStr);
    }
    
    return 0;
}

function IsAuthenticated() {
    if (Sync.getValue("sessionToken") != "") {
        return 1;
    }
    return 0;
}

function GetFolderList(list) {
    local sessionToken = Sync.getValue("sessionToken");
    nm.setUrl("https://www.mediafire.com/api/1.5/folder/get_content.php");
    nm.setReferer("https://app.mediafire.com/");
    nm.addQueryParam("session_token", sessionToken);
    nm.addQueryParam("response_format", "json");
    nm.addQueryParam("folder_key", "myfiles");
    nm.addQueryParam("content_type", "folders");
    nm.addQueryParam("chunk", 1);
    nm.addQueryParam("chunk_size", 100);
    nm.addQueryParam("details", "yes");
    nm.addQueryParam("order_direction", "asc");
    nm.addQueryParam("order_by", "name");
    nm.addQueryParam("filter", "");
    nm.doPost("");

    if (nm.responseCode() == 200) {
        local t = ParseJSON(nm.responseBody());
        if ("response" in t) {
            local folders = t.response.folder_content.folders;
            foreach(item in folders) {
                local folder = CFolderItem();
                folder.setId(item.folderkey);
                folder.setTitle(item.name);
                folder.setSummary(item.description);
                list.AddFolderItem(folder);
            }
            return 1;
        } else {
            WriteLog("error", "[mediafire.com] Failed to load folder list, response code=" + nm.responseCode());
        }
    } else {
        WriteLog("error", "[mediafire.com] Failed to load folder list, response code=" + nm.responseCode());
    }
    return 0;
}

function _GetFileInfo(fileid) {
    local sessionToken =  Sync.getValue("sessionToken");
    nm.setUrl("https://www.mediafire.com/api/file/get_info.php");
    nm.setReferer("https://www.mediafire.com/");
    nm.addQueryParam("session_token", sessionToken);
    nm.addQueryParam("response_format", "json");
    nm.addQueryParam("quick_key", fileid);
    nm.doPost("");

    if (nm.responseCode() == 200) {
        local t = ParseJSON(nm.responseBody());
        if (!("response" in t) || t.response.result == "error") {
            WriteLog("error", "[mediafire.com] Failed to get file info");
        }
    } else {
        WriteLog("error", "[mediafire.com] Failed to get file info, responseCode=" +  nm.responseCode());
    }
}

function _Check(folderkey, filebase, filesize, filehash) {
    local sessionToken = Sync.getValue("sessionToken");
    local upload = {
        filename = filebase,
        folder_key = folderkey,
        size = filesize,
        hash = filehash,
        resumable = "yes",
        preemptive = "yes"
    };
    local uploads = [
        upload
    ];
    local url = "https://www.mediafire.com/api/1.5/upload/check.php";
    nm.addQueryParam("uploads", ToJSON(uploads)); 
    nm.addQueryParam("response_format", "json");
    nm.addQueryParam("session_token", sessionToken);
    nm.setUrl(url);
    nm.doPost("");

    if (nm.responseCode() == 200) {
        local t = ParseJSON(nm.responseBody());
        if (t.response.result == "Success") {
            return t.response.resumable_upload.unit_size;
        } 
    }
    return 1024 * 1024;
}

function _ChunkedUpload(sessionToken, folderId, FileName, fileNameBase, fileSize, fileHash, options) {
    local chunkSize = _Check(folderId, fileNameBase, fileSize, fileHash).tointeger();
    local chunkCount = ceil(fileSize.tofloat() / chunkSize).tointeger();
    local lastChunk = chunkCount - 1;
   
    for (local i = 0; i < chunkCount; i++) {
        local offset = i * chunkSize;
        local currentChunk = min(chunkSize, fileSize-offset).tointeger();
        local chunkHash = Sha256FromFile(FileName, offset, currentChunk);
        local chunk = i == lastChunk ? currentChunk: chunkSize;

        nm.setUrl("https://www.mediafire.com/api/1.5/upload/resumable.php?folder_key=" + nm.urlEncode(folderId) + "&response_format=json&session_token=" + nm.urlEncode(sessionToken))
        nm.addQueryHeader("Content-Type", "application/octet-stream");
        nm.setReferer("https://app.mediafire.com/");
        nm.addQueryHeader("X-Filesize", currentChunk);
        nm.addQueryHeader("X-Filetype",  "");
        nm.addQueryHeader("X-Filehash", fileHash);
        nm.addQueryHeader("X-Filename", fileNameBase);
        nm.addQueryHeader("X-Unit-Hash", chunkHash);
        nm.addQueryHeader("X-Unit-Size", chunk);
        nm.addQueryHeader("X-Unit-Id", i);
        nm.setChunkOffset(offset);
        nm.setChunkSize(currentChunk);
        nm.setMethod("POST");
        nm.doUpload(FileName, "");

        if (nm.responseCode() == 200) {
            local t3 = ParseJSON(nm.responseBody());
            if (t3.response.doupload.result != "0") {
                WriteLog("error", "[mediafire.com] Chunk #" + i +" upload failed");
                break;
            } else {
                if (i == lastChunk && t3.response.resumable_upload.all_units_ready=="yes") {
                    local key = t3.response.doupload.key;
                    return _PollUpload(sessionToken, key, fileNameBase, options);
                }
            }
        } else {
            WriteLog("error", "[mediafire.com] Chunk #" + i +" upload failed, responseCode=" +  nm.responseCode());
            break;
        }
    }
    return 0;
}

function _PollUpload(sessionToken, key, fileNameBase, options) {
    nm.setUrl("https://www.mediafire.com/api/upload/poll_upload.php");
    nm.setReferer("https://app.mediafire.com/");
    nm.addQueryParam("session_token", sessionToken);
    nm.addQueryParam("key", key);
    nm.addQueryParam("filename", fileNameBase);    
    nm.addQueryParam("response_format", "json");
    nm.doPost("");
    if (nm.responseCode() == 200) {
        local t2 = ParseJSON(nm.responseBody());
        if ("response" in t2) {
            options.setViewUrl("https://www.mediafire.com/view/" +  t2.response.doupload.quickkey);
            return 1;
        }
        
        //_GetFileInfo(key);
        //options.setViewUrl("https://www.mediafire.com/view/" + key + "/" + nm.urlEncode(fileNameBase) + "/file");
        //options.setDirectUrl("https://www.mediafire.com/file/" + key + "/" + nm.urlEncode(fileNameBase) + "/file");
        
    } else {
        WriteLog("error", "[mediafire.com] Failed to upload, responseCode=" +  nm.responseCode());
    }
    return 0;
}

function UploadFile(FileName, options) {
    local sessionToken =  Sync.getValue("sessionToken");
    local fileSize = GetFileSize(FileName);
    local fileNameBase = ExtractFileName(FileName);
    local mimeType = GetFileMimeType(FileName);

    local folderId = options.getFolderID();
    if (folderId == "") {
        folderId = "myfiles";
    }        
    local fileHash = Sha256FromFile(FileName, 0, 0);

    if (fileSize > 4000000) {
        return _ChunkedUpload(sessionToken, folderId, FileName, fileNameBase, fileSize, fileHash, options);
        // Chunked upload
    } else {
        nm.setUrl("https://www.mediafire.com/api/1.5/upload/simple.php?folder_key=" + nm.urlEncode(folderId) + "&response_format=json&session_token=" 
            +  nm.urlEncode(sessionToken));
        nm.addQueryHeader("X-Filename", fileNameBase);
        nm.addQueryHeader("X-Filesize", fileSize);
        nm.addQueryHeader("X-Filehash", fileHash);
        nm.addQueryHeader("Content-Type", "application/octet-stream");
        nm.setReferer("https://app.mediafire.com/");
        nm.setMethod("POST");
        nm.doUpload(FileName, "");

        if (nm.responseCode() == 200) {
            local t = ParseJSON(nm.responseBody());
            if ("response" in t && t.response.doupload.result == "0") {
                local key = t.response.doupload.key;
                return _PollUpload(sessionToken, key, fileNameBase, options);
                
            } else {      
                WriteLog("error", "[mediafire.com] Failed to upload" );
            }
        } else {
            WriteLog("error", "[mediafire.com] Failed to upload, responseCode=" +  nm.responseCode());
        }
    }
    return 0;
}