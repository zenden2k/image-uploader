/*
	Utilites for comfort string processing.
*/

include("Utils/EncDecd.nut");

/*
	Replace @pattern with @replace_with in string @str
	
	input parameters:
	string str - data for processing
	string pattern - what find
	string replace_with - what paste

	Result:
		returns @data string with replaced @pattern on @replace_with
*/
function strReplace(str, pattern, replace_with)
{
	local resultStr = str;	
	local res;
	local start = 0;

	while( (res = resultStr.find(pattern,start)) != null ) {	

		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
		start = res + replace_with.len();
	}
	return resultStr;
}


/*
	Extract sring from json value and decode it to human readable format.
	
	Input:
		JSON encoded string
		
	Output:
		normal, human readable string
	
	@dev: parseJSON doing the same, while processing data to table?
*/
function unescape_json_string(data) {
    local tmp;

    local ch = 0x0424;
	local result = data;
	local ex = regexp("\\\\u([0-9a-fA-F]{1,4})");
	local start = 0;
	local res = null;
	for(;;) {
		res = ex.capture(data, start);
		local resultStr = "";
		if (res == null){
			break;
		}
			
		resultStr = data.slice(res[1].begin, res[1].end);
		ch = hex2int(resultStr);
		start = res[1].end;
		 if(ch>=0x00000000 && ch<=0x0000007F)
			tmp = format("%c",(ch&0x7f));
		else if(ch>=0x00000080 && ch<=0x000007FF)
			tmp = format("%c%c",(((ch>>6)&0x1f)|0xc0),((ch&0x3f)|0x80));
		else if(ch>=0x00000800 && ch<=0x0000FFFF)
		   tmp= format("%c%c%c",(((ch>>12)&0x0f)|0xe0),(((ch>>6)&0x3f)|0x80),((ch&0x3f)|0x80));
			result = strReplace( result, "\\u"+resultStr, tmp);
   
	}

    return result;
}
