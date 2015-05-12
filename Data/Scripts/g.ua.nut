function  ShortenUrl(url, options)
{	
	nm.addQueryHeader("X-Requested-With", "XMLHttpRequest");
	nm.doGet("http://g.ua/?u=" + nm.urlEncode(url));

	if ( nm.responseCode() == 200 ) {
		local link = ParseJSON(nm.responseBody()).shortLink;
		options.setDirectUrl(link);
		return 1;
	}

	return 0;
}