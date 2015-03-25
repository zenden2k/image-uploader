/*
	Various encoding and decoding functions
*/

/*
	Encode input @input string into base64 encoding and return encoded data as string
*/
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

/*
	Convert hexdecimal string into integer value
*/
function hex2int(str){
	local res = 0;
	local step = 1;
	for( local i = str.len() -1; i >= 0; i-- ) {
		local val = 0;
		local ch = str[i];
		if ( ch >= 'a' && ch <= 'f' ) {
			val = 10 + ch - 'a';
		}
		else if ( ch >= '0' && ch <= '9' ) {
			val = ch - '0';
		}
		res += step * val;
		step = step * 16;
	}
	return res;
}
