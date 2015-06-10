function  ShortenUrl(url, options)
{	
	nm.setUrl("https://www.googleapis.com/urlshortener/v1/url?key=AIzaSyBtHuY574sbFT3NKQjQRwBw2h7Fw_EpIsg");
	nm.addQueryHeader("Content-Type", "application/json");

	local postData = {
		longUrl = url
	};
	nm.doPost(ToJSON(postData));


	local id = ParseJSON(nm.responseBody()).id;

	options.setDirectUrl(id);
	options.setViewUrl(id);
	return 1;
}