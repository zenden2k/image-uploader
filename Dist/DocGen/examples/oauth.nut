const CLIENT_ID = "test_client";

function Authenticate() {
    if (ServerParams.getParam("token") != "") {
        return ResultCode.Success;
    }
    local login = ServerParams.getParam("Login");
    local scope = "offline_access files.readwrite";

    // This is an example of OAuth authorization
    // Opening the link in the embedded web browser
    local redirectUri = "https://test.com/callback";
    local url = "https://example.com/oauth20_authorize?client_id=" + CLIENT_ID + "&scope="+ nm.urlEncode(scope) + "&response_type=code&redirect_uri=" + nm.urlEncode(redirectUri)
    local browser = CWebBrowser();
    browser.setTitle(tr("example.browser.title", "Example.com authorization"));
    browser.setOnUrlChangedCallback(function(data) {
        local reg = CRegExp("^" + redirectUrlEscaped, "");
        if (reg.match(data.url) ) {
            local br = data.browser;
            local regError = CRegExp("error=([^&]+)", "");
            if ( regError.match(data.url) ) {
                WriteLog("warning", regError.getMatch(1));
            } else {
                local codeRegexp = CRegExp("code=([^&]+)", "");
                if ( codeRegexp.match(data.url) ) {
                    authCode = codeRegexp.getMatch(1);
                }
            }
            br.close();
        }
    }, null);
    browser.navigateToUrl(url);
    browser.showModal();

    // Another example: opening the link in the default web browser and launching a web server
    local server = WebServer();
    local confirmCode = "";

    server.resource("^/$", "GET", function(d) {
        local responseBody = "";
        if ("code" in d.queryParams){
            confirmCode = d.queryParams.code;
            responseBody = "<h1>" + tr("example.oauth.title", "Authorization") + "</h1><p>" + tr("example.oauth.success", "Success! Now you can close this page.")+"</p>";
        } else {
            responseBody = "<h1>" + tr("example.oauth.title", "Authorization") + "</h1><p>" + tr("example.oauth.success", "Failed to obtain confirmation code") + "</p>";
        }

        return {
            responseBody = responseBody,
            stopDelay = 500 // server shutdown after 500 ms
        };
    }, null);

    local port = server.bind(0); // random port

    local redirectUrl = "http://127.0.0.1:" + port + "/";

    ShellOpenUrl(url); //  opening the link in the default web browser
    server.start();

    local confirmCode = code;
    if (confirmCode == "") {
        WriteLog("error", "Cannot authenticate without confirm code");
        return ResultCode.Failure;
    }

    nm.setUrl("https://example.com/common/oauth2/v2.0/token");
    nm.addPostField("code", confirmCode);
    nm.addPostField("client_id", clientId);
    nm.addPostField("redirect_uri", redirectUri);
    nm.addPostField("grant_type", "authorization_code");
    nm.doPost("");
    if (nm.responseCode() != 200) {
        WriteLog("error", "[example.com] Authentication failed. Response code: " + nm.responseCode());
        return ResultCode.Failure;
    }
    local data = nm.responseBody();
    local t = ParseJSON(data);

    if ("access_token" in t) {
        local timestamp = time();
        ServerParams.setParam("token", t.access_token);
        ServerParams.setParam("expiresIn", t.expires_in);
        ServerParams.setParam("refreshToken", t.refresh_token);
        ServerParams.setParam("tokenTime", timestamp.tostring());
        return ResultCode.Success;
    } else {
        WriteLog("error", "[example.com] Authentication failed");
    }
    return ResultCode.Failure;
}