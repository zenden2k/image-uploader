/*
	Debug utilites
*/

/*
	write message to log or to stdout if first try failed.
	
	Input:
		string type - message type, according to the dev. documentation, it can be "error" or "warning"
	
	Out:
		no result.
*/
function _WriteLog(type,message) {
	try {
		WriteLog(type, message);
	} catch (ex ) {
		print(type + " : " + message);
	}
}