/*
	Common utilites for regular expressions comort using.
*/

/*
	Simple get firch match from @data using @regStr expression from @start position in string @data
	
	Input parameters:
	 string data - procesing data
	 string regStr - regular expression(not PRCE)
	 integer start - starting char position of @data from work will be begin.
	
	return string value.
*/
function regex_simple(data,regStr,start)
{
	local ex = regexp(regStr);
	local res = ex.capture(data, start);
	local resultStr = "";
	if(res != null){	
		resultStr = data.slice(res[1].begin, res[1].end);
	}
	return resultStr;
}