
function PreUpload(task,reserved) 
{
	
	//print(task.serverName());
	if ( task.type() == "TypeFile") {
		local ext = GetFileExtension(task.getFileName());
		if ( ext.tolower() == "png" ) {
			local displayName = ExtractFileNameNoExt(task.getDisplayName());
			local tempName = GetTempDirectory() + displayName + "_" + GetCurrentThreadId() + "_" + random()%50000 + ".png";
			if ( CopyFile(task.getFileName(), tempName, true) ) {
				task.setStatusText("Optimizing png...");
				task.addTempFile(tempName);
				local process = Process("optipng.exe", true);
				process.setArguments([tempName]);
				process.setHidden(true);
				//process.setCaptureOutput(true);
				//process.launchInShell("optipng.exe \""+tempName+"\"");
				process.start();
				//local res = process.readOutput();

				local exitCode = process.waitForExit();
				if ( exitCode == 0 ) { // Success
					task.setFileName(tempName);
					task.setDisplayName( displayName +".png")
					
					local parent = task.parentTask();
					if ( parent == null || parent.isNull() ) {
						//local newTask = FileUploadTask(tempName, displayName+".png");
						//task.addChildTask(newTask);
						//print(parent.serverName());
					}
	
				} else {
					WriteLog("error", "Optipng returned code " + exitCode);
					return false;
				}
			} else {
				WriteLog("error", "Cannot copy file to " + tempName);
				return false;
			}
			
		}
	}
	
	return true;
}

function PostUpload(task,reserved) {
	return true;
}