/*
	Utils for script proposes.
*/

/*
	Tries call internal sllep, if it failed, make own sleep analog.
*/
function TrySleep() {
	try {
		sleep(300);
	} catch ( ex ) {
		local retTime = time() + 1;     
		while (time() < retTime);  
	}
}