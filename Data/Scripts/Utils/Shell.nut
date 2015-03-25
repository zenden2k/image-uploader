/*
	Common utils for working with OS shell, e.g. open url in web or show inputBox or messageBox
*/

/*
	tries open @url string in OS shell by default web-browser.
*/
function openUrl(url) {
	try{
		return ShellOpenUrl(url);
	}catch(ex){}

	system("start "+ strReplace(url,"&","^&") );
}

/*
	create input dialog to interact with user, and return to script, what user want.
*/
function inputBox(prompt, title) {
	try {
		return InputDialog(prompt, "");
	}catch (e){}
	local tempScript = "%temp%\\imguploader_inputbox.vbs";
	prompt = strReplace(prompt, "\n", "\" ^& vbCrLf ^& \"" );
	local tempOutput = getenv("TEMP") + "\\imguploader_inputbox_output.txt";
	local command = "echo result = InputBox(\""+ prompt + "\", \""+ title + "\") : Set objFSO=CreateObject(\"Scripting.FileSystemObject\") : Set objFile = objFSO.CreateTextFile(\"" + tempOutput + "\",True) : objFile.Write result : objFile.Close  > \"" + tempScript + "\"";
	system(command);
	command = "cscript /nologo \"" + tempScript + "\"";// > \"" + tempOutput + "\"";*/
	system(command);
	local res = readFile(tempOutput);
	system("rm \""+ tempOutput + "\"");
	return res;
}


/*
	Create message box as user notifications
	
	Input:
		string text - what you want display to user.
		
	Return true if showing successfull, otherwise return false.
	
	@2dev: 1.3.x internal mesageBox call need here.
*/

function msgBox(text) {
	try {
		DebugMessage(text, false);
		return true;
	}catch(ex) {
	}
	local tempScript = "%temp%\\imguploader_msgbox.vbs";
	system("echo Set objArgs = WScript.Arguments : messageText = objArgs(0) : MsgBox messageText > \"" + tempScript + "\"");
	system("cscript \"" + tempScript + "\" \"" + text + "\"");
	system("del /f /q \"" + tempScript + "\"");
	return true;
}
