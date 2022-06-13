function ShortenUrl(url, options) {
    local apiKey = ServerParams.getParam("Login");

    if (apiKey == "") { // Anonymous upload
        nm.setUrl("https://link.ac/");
        nm.addQueryParam("qmode", "cutlink");
        nm.addQueryParam("needurl", "");
        nm.addQueryParam("url", url);
        nm.doPost("");

        if (nm.responseCode() == 200) {
            local data = nm.responseBody();
            local reg = CRegExp("\\[URL\\](.+?)\\[/URL\\]","i");
            if (!reg.match(data) ) {
                return 0;
            }

            local shortUrl = reg.getMatch(1);

            if (shortUrl != "") {
                options.setDirectUrl(shortUrl);
                return 1;
            }
        }
    } else {
        nm.doGet("https://link.ac/?key=" + nm.urlEncode(apiKey) +"&url="+ nm.urlEncode(url));
        if (nm.responseCode() == 200) {
            local response = nm.responseBody();
            local reg = CRegExp("^https:\\/\\/link\\.ac\\/[\\w]+$","i");
            if (reg.match(response)) {
                options.setDirectUrl(response);
                return 1;
            } else {
                WriteLog("error", "link.ac error: " + response);
            }
        }
    }
    return 0;
}

