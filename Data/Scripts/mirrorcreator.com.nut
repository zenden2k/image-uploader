/* by Alex_Qwerty */

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


function base64Encode(input) {
	local keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    local output = "";
    local chr1, chr2, chr3, enc1, enc2, enc3, enc4;
    local i = 0;
	local len = input.len() ;

    while ( i < len ) {

        chr1 = input[i++];
		if ( i< len) {
			chr2 = input[i++];
		} else {
			chr2 = 0;
		}
		if ( i < len ) {
			chr3 = input[i++];
		} else {
			chr3 = 0;
		}

        enc1 = chr1 >> 2;
        enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
        enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
        enc4 = chr3 & 63;

        if (chr2 == 0) {
            enc3 = enc4 = 64;
        } else if (chr3 == 0) {
            enc4 = 64;
        }
		//print("enc1=" + enc1 + " enc2=" + enc2 + " enc3=" + enc3);
        output = output + format("%c", keyStr[enc1] ) +
			format ( "%c", keyStr[enc2])
			+ format("%c", keyStr[enc3])
			+ format("%c", keyStr[enc4]);
    }

    return output;
}


function  UploadFile(FileName, options)
{
	local fid = format("%c%c%c%c%c%c", rand()%26+'A', rand()%26+'A', rand()%26+'A', rand()%26+'A', rand()%26+'A', rand()%26+'A');
	local fsize = GetFileSize(FileName);
	local fn = ExtractFileName(FileName);
	local url = "http://www.mirrorcreator.com/fnvalidator.php?fn=" + fn + "%20(" + fsize + ");&fid=" + fid + ";"
	nm.addQueryHeader("X-Requested-With", "XMLHttpRequest");
	nm.doGet(url);

	local fnv = nm.responseBody();

	nm.setUrl("http://www.mirrorcreator.com/uploadify/uploadify.php");
	nm.addQueryHeader("X-Requested-With", "");
	nm.addQueryHeader("User-Agent", "Shockwave Flash");
	nm.addQueryParam("Filename", fn);
	nm.addQueryParam("folder","/uploads");
	nm.addQueryParamFile("Filedata", FileName,fn, "");
	nm.addQueryParam("Upload","Submit Query");
	nm.doUploadMultipartData();
	local data = nm.responseBody();

	local fn2 = regex_simple(data, "\"fileName\":\\s*\"([^\"]+)\"", 0);
	local pd = base64Encode( fn2 + "#0#"+fsize+";0;@e@#H#solidfiles;sendmyway;rghost;gett;turbobit;sharebeast;hugefiles;uptobox;filesfm;datafilehost;uppit;userscloud;#P##SC#" );
	nm.addQueryHeader("User-Agent", "");
	nm.doGet("http://www.mirrorcreator.com/process.php?data=" + pd);
	local data2 = nm.responseBody();

	url = regex_simple(data2, "(http://mir.cr/[0-9A-Z]+)", 0);
	//url = regex_simple(data2, "\"(http://www.mirrorcreator.com/files/[^\"]+)\"", 0);

	options.setViewUrl(url);

	return 1;
}
