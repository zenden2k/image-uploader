function reg_replace(str, pattern, replace_with)
{
	local resultStr = str;	
	local res;
	local start = 0;
	while(res = resultStr.find(pattern,start)){	
		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
		start = res + replace_with.len();
	}
	return resultStr;
}


function  UploadFile(FileName, options)
{
	local newFilename = ExtractFileName(FileName);
	local directory = ServerParams.getParam("directory");
	local targetFile = directory + newFilename;
	
	if ( FileExists(targetFile) ) {
		local ext = GetFileExtension(newFilename);
		newFilename = ExtractFileNameNoExt(newFilename) + "_" + random() + (ext == "" ? "": ("."+ext));
		targetFile = directory + newFilename;
	}
	local res = CopyFile(FileName, targetFile, true);
	if ( !res ) {
		print("Copying file from "+FileName+" to " + targetFile " failed");
		return 0;
	}
	
	local downloadUrl =  ServerParams.getParam("downloadUrl");
	if (downloadUrl == "") {
		print("downloadUrl parameter should not be empty");
		return 0;
	}
	local encodedFileName = newFilename;
	if ( downloadUrl.find("://") != null ) {
		newFilename = reg_replace(nm.urlEncode(newFilename),"%2E",".");
	}
	
	options.setDirectUrl(downloadUrl + newFilename);

	return 1;
}

function GetServerParamList()
{
	local a =
	{
		directory   = "Directory"
		downloadUrl = "Download path (ftp or http)"
	}
	return a;
}