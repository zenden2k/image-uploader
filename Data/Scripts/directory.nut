function reg_replace(str, pattern, replace_with)
{
	local resultStr = str;	
	local res;
	local start = 0;

	res = resultStr.find(pattern,start);
	while( (res = resultStr.find(pattern,start)) != null ) {	

		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
		start = res + replace_with.len();
	}
	return resultStr;
}


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
		encodedFileName = reg_replace(nm.urlEncode(newFilename),"%2E",".");
	}  
		
		
	options.setDirectUrl(downloadUrl + encodedFileName);
		
	if ( downloadUrl.find("\\\\") == 0 ) {
		local convertedUrl = "file:///" + reg_replace(downloadUrl,"\\","/") + reg_replace(nm.urlEncode(newFilename),"%2E",".");
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