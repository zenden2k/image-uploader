/**
@file
@section Implement Functions to implement
You have to implement at least one function — \ref UploadFile.<br>
If your service supports authentication, you have to implement \ref Authenticate function.
If you want to support album listing/creating/modifying, you have to implement also \ref GetFolderList, \ref CreateFolder, 
 \ref ModifyFolder, \ref GetFolderAccessTypeList</i>.</p>
*/

/**
Required function for server Type="image" or Type="file".
@return 1 - success,<br>
0 - fail<br/>
-1 - fail and abort upload  (for example, authorization failed, this value supported since v.1.3.1)
*/
int UploadFile(string pathToFile, UploadParams params);

int ShortenUrl(string url, UploadParams params);
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
function GetServerParamList() {
	return {
		useWebdav = "Use WebDav",
		someOption = {
			title = "Some option",
			type = "text"
		},
		useOAuth = {
			title = "Use OAUth",
			type = "boolean"
		},
		expiration = {
            title = "Expiration",
            type = "choice",
            items = [
                {
                    id = "",
                    label = Never"
                },
                {
                    id = "PT5M",
                    label = "15 minutes"
                },
                {
                    id = "PT30M",
                    label = "30 minutes"
                },
                {
                    id = "PT1H",
                    label = "1 hour"
                }
			]
		},
		privateKeyPath = {
            title = "Private key path",
            type = "filename"
        } 
	};
}
@endcode
*/
table GetServerParamList();

/**
@return 1 - is authenticated,<br>
0 - is not authenticated<br/>
*/
int IsAuthenticated();

/**
@return 1 - success,<br>
0 - failure<br/>
*/
int Authenticate();

/**
@since 1.3.3
@return 1 - success,<br>
0 - failure<br/>
*/
int RefreshToken();

/**
@return 1 - success,<br>
0 - fail<br/>
*/
int DoLogout();

