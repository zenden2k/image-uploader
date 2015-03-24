/*
	Some math utils.
*/

/*
 generate random integer number and returns it as string.
*/
function generateNonce() {
	local res = "";
	res += format("%d%d%d", random(2000), random(2000), random(2000));
	return res;
}

/*
	returns true if a if a < b and b if b > a.
	
	input:
	integer a,b - what you wand compare
*/

function min(a,b) {
	return a < b ? a : b;
}
