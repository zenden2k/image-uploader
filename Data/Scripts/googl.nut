function  ShortenUrl(url, options)
{	
	nm.setUrl("https://www.googleapis.com/urlshortener/v1/url?key=AIzaSyBtHuY574sbFT3NKQjQRwBw2h7Fw_EpIsg");
	nm.addQueryHeader("Content-Type", "application/json");

	local postData = {
		longUrl = url
	};
	nm.doPost(ToJSON(postData));

    if (nm.responseCode() == 200) {
        local jsonData = ParseJSON(nm.responseBody());
        if ( "id" in jsonData) {
            local id = jsonData.id;

            options.setDirectUrl(id);
            options.setViewUrl(id);
            return 1;
        }
    } else {
        WriteLog("error", "goo.gl: invalid response code " + nm.responseCode());
    }
    return 0;
}