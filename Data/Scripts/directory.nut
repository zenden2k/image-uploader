include("Utils/RegExp.nut");
include("Utils/String.nut");

function  UploadFile(FileName, options)
{
	local newFilename = ExtractFileName(FileName);
	local directory = ServerParams.getParam("directory");
	local convertUncPath = 0;
	try {
		 convertUncPath = ServerParams.getParam("convertUncPath").tointeger();
	} catch(ex){}	

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
		encodedFileName = strReplace(nm.urlEncode(newFilename),"%2E",".");
	}  
		
		
	options.setDirectUrl(downloadUrl + encodedFileName);
		
	if ( downloadUrl.find("\\\\") == 0 ) {
		local convertedUrl = "file:///" + strReplace(downloadUrl,"\\","/") + strReplace(nm.urlEncode(newFilename),"%2E",".");
		if ( convertUncPath == 1) {
			options.setDirectUrl(convertedUrl);
		} else {

			options.setViewUrl(convertedUrl);
		}
	}

	return 1;
}

function GetServerParamList()
{
	local a =
	{
		directory   = "Directory"
		downloadUrl = "Download path (ftp or http)",
		convertUncPath = "Convert UNC path \"\\\\\" to file://///"
	}
	return a;
}