const UPLOAD_URL = "http://example.com/upload.php";
test <- "example"; // global variable

// This is the function that performs the upload of the file
// @param string pathToFile 
// @param UploadParams options
// @return int - success(1), failure(0)

function UploadFile(pathToFile, options) {
    local task = options.getTask().getFileTask();
    nm.setUrl(UPLOAD_URL);
    nm.addQueryParamFile("file", pathToFile, task.getDisplayName(), GetFileMimeType(pathToFile));
    nm.addQueryParam("submit", "Upload file!");
    nm.doUploadMultipartData();

    if (nm.responseCode() != 200) {
        return ResultCode.Failure;
    }

    local directUrl = _RegexSimple(nm.responseBody(), "\\[IMG\\](.+)\\[/IMG\\]",0);
    
    if (directUrl == "") {
        return ResultCode.Failure;
    }

    options.setDirectUrl(directUrl);

    return ResultCode.Success;
}

// Helper function that simplifies working with regular expressions
// @param string data - the string we are looking in
// @param string regStr - regular expression, in the format supported by the standard squirrel language library.
//      http://www.squirrel-lang.org/doc/sqstdlib2.html#d0e2580
//      This format does not support some of the features of the PCRE format used in servers.xml.
// @param int start - starting position
// @return string - returns text captured by the first subpattern.
//
function _RegexSimple(data,regStr,start) {
    local ex = regexp(regStr);
    local res = ex.capture(data, start);
    local resultStr = "";
    if(res != null){    
        resultStr = data.slice(res[1].begin, res[1].end);
    }
    return resultStr;
}

// Authenticating on remote server (optional function)
// @var CFolderList list
// @return int - success(1), failure(0) 
function Authenticate() {
    // TODO: Your code
    return ResultCode.Success;
}


// Optional function:
// Retrieving folder (album) list from server
// @var CFolderList list
// @return int - success(1), failure(0) 
//
function GetFolderList(list) {
    // TODO: Your code
    return ResultCode.Success;
}
 
 
// Create an folder or an album
// @var CFolderItem parentAlbum
// @var CFolderItem album
// @return int - success(1), failure(0)
//
function CreateFolder(parentAlbum,album) {
    // TODO: Your code
    return ResultCode.Success;
}
 
// Modify a folder or an album (update name, description)
// @var CFolderItem album
// @return int - success(1), failure(0) 
//
function ModifyFolder(album) {
    // TODO: Your code
    return ResultCode.Success;
}
 
// A function that returns a list of types of access restrictions to an album:
// private, public, only for friends, etc.
// @return array 
function GetFolderAccessTypeList() {
    return ["Private", "Public"];
}
