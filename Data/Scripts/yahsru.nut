function ShortenUrl(url, options) {
	local dummy = "value=\"";
	local surl = "";
	nm.setUrl("http://yahs.ru/links/page");
	nm.addQueryParam("link_text2", url);
	nm.addQueryParam("links_butt", "create");
	nm.doPost("");
	if (nm.responseCode() == 200) {
		local sHTML = nm.responseBody();
		local indx = 0;
		indx = sHTML.find("<input id=\"message_link\" ", 0);
		if (indx > 0) {
			local indx_val = sHTML.find(dummy, indx);
			if (indx_val > 0) {
				indx_val += dummy.len();
				local next_val = sHTML.find("\"", indx_val);
				local s = sHTML.slice(indx_val, next_val);
				if (s != "")
					surl = s;
			};
		};

		if (surl == "")
			return 0;

		options.setDirectUrl(surl);
		options.setViewUrl(surl);
		return 1;
	} else
		return 0;
}