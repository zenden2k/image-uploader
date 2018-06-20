/**
@file
@section Implement Functions to implement
You have to implement at least one function — <code>UploadFile</code>.<br>
If you want to support album listing/creating/modifying, you have to implement also <code>GetFolderList</code>, <code>CreateFolder</code>, 
<code>ModifyFolder</code>, <code>GetFolderAccessTypeList</code></i>.</p>
*/

/**
Required function for server Type="image" or Type="file".
@return 1 - success,<br>
0 - fail<br/>
-1 - fail and abort upload  (for example, authorization failed, this value supported since v.1.3.1)
*/
int UploadFile(string pathToFile, CIUUploadParams params);

int ShortenUrl(string url, CIUUploadParams params);
int GetFolderList(CFolderList folderList);
int CreateFolder(CFolderItem parent, CFolderItem folder);
int ModifyFolder(CFolderItem folder);
/**
Function that returns a list of the types of access restrictions to the album:
  private, public, friends only, etc.
@code{.nut}
function GetFolderAccessTypeList()
{
    return ["Public", "Private"];
}
@endcode
*/
array GetFolderAccessTypeList();

/**
@code{.nut}
function GetServerParamList()
{
	return 
	{
		useWebdav = "Use WebDav",
		token = "Token",
		enableOAuth ="enableOAuth",
		tokenType = "tokenType",
		PrevLogin = "PrevLogin",
		OAuthLogin = "OAuthLogin"
		
	};
}
@endcode
*/
table GetServerParamList();
