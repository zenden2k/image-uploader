test <- "example"; // global variable

// Function which is actually doing upload of a file
// @param string pathToFile 
// @param UploadParams options
// @return int - success(1), failure(0)

function  UploadFile(pathToFile, options)
{
    nm.setUrl("http://example.com/upload.php");
    nm.addQueryParamFile("file", pathToFile, ExtractFileName(pathToFile),"");
    nm.addQueryParam("submit", "Upload file!");
    nm.doUploadMultipartData();

    local response = nm.responseBody(); // 'local' it's like javascript's 'var' but only for local variables
    local directUrl = regex_simple(response, "\\[IMG\\](.+)\\[/IMG\\]",0);
    
    options.setDirectUrl(directUrl);

    return 1; //SUCCESS
}

 
// Helper function that simplifies working with regular expressions
// @param string data - the string we are looking in
// @param string regStr - regular expression, in the format supported by the standard squirrel language library.
//      http://www.squirrel-lang.org/doc/sqstdlib2.html#d0e2580
//      This format does not support some of the features of the PCRE format used in servers.xml.
// @param int start - starting position
// @return string - returns text captured by the first subpattern.
//
function regex_simple(data,regStr,start)
{
    local ex = regexp(regStr);
    local res = ex.capture(data, start);
    local resultStr = "";
    if(res != null){    
        resultStr = data.slice(res[1].begin, res[1].end);
    }
    return resultStr;
}

/** 
* Optional function:
* Retreving folder (album) list from server
* @var CFolderList list
* @return int - success(1), failure(0) 
**/
function GetFolderList(list)
{
    // TODO: Your code
    return 1; //SUCCESS
}
 
/** 
* Create an folder or an album
* @var CFolderItem parentAlbum
* @var CFolderItem album
* @return int - success(1), failure(0)
**/
function CreateFolder(parentAlbum,album)
{
    // TODO: Your code
    return 1; //SUCCESS
}
 
// Modify a folder or an album (update name, description)
// @var CFolderItem album
// @return int - success(1), failure(0) 
//
function ModifyFolder(album)
{
    // TODO: Your code
    return 1; //SUCCESS
}
 
// A function that returns a list of types of access restrictions to an album:
// private, public, only for friends, etc.
// @return array 
function GetFolderAccessTypeList()
{
    return ["Private", "Public"];
}
