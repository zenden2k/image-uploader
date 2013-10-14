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

function  ShortenUrl(url, options)
{	
	nm.setUrl("https://www.googleapis.com/urlshortener/v1/url?key=AIzaSyBtHuY574sbFT3NKQjQRwBw2h7Fw_EpIsg");
	nm.addQueryHeader("Content-Type", "application/json");

	nm.addQueryParam("url", url);
	nm.doPost("{\"longUrl\": \""+ JsonEscapeString(url) + "\"}");

	print(nm.responseBody());

	local id = regex_simple(nm.responseBody(), "id\" *: *\"(.+)\"", 0);

	options.setDirectUrl(id);
	options.setViewUrl(id);
	return 1;
}