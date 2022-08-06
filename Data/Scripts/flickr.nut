appKey <- "2fbf2be64367abf2515f8b5494a5bc43";
appSecret <- "116fdf4bcac0cf76";
accessType <- "app_folder";

authStep1Url <- "https://www.flickr.com/services/oauth/request_token";
authStep2Url <- "https://www.flickr.com/services/oauth/access_token";

function regex_simple(data, regStr, start) {
    local ex = regexp(regStr);
    local res = ex.capture(data, start);
    local resultStr = "";
    if (res != null){
        resultStr = data.slice(res[1].begin, res[1].end);
    }
    return resultStr;
}

function generateNonce() {
    local res = "";
    res += format("%d%d%d", random()%2000, random()%2000, random()%2000);
    return res;
}

function custom_compare(item1, item2) {
    local a = item1.a+"="+item1.b;
    local b = item2.a+"="+item2.b;
    if(a>b) return 1;
    else if(a<b) return -1;
    return 0;
}

function checkResponse() {
    if ( nm.responseCode() == 0 || (nm.responseCode() >= 400 && nm.responseCode() <= 499)) {
        WriteLog("error", "Response code " + nm.responseCode() + "\r\n" + nm.errorString() );
        return 0;
    }
    return 1;
}

function signRequest(method, url, params, token, tokenSecret) {
    if ( token != "" ) {
        params.append({a="oauth_token",b=token});
    }
    params.sort(custom_compare);

    local normalizedRequest = "";

    normalizedRequest += "&" + url_encode(url);
    local tableLen = params.len();
    local baseString = "";
    for ( local i=0; i< tableLen; i++ ) {
        params[i].b = (params[i].b);
        if ( i > 0 ) {
            baseString += "&" ;
        }
        baseString += params[i].a + "=" + url_encode(params[i].b);
    }

    normalizedRequest = method+"&"+url_encode(url) + "&" + url_encode(baseString) ;

    local oauth_signature = hmac_sha1(appSecret + "&" + tokenSecret, normalizedRequest, true);
    params.append({a="oauth_signature",b=oauth_signature});
    params.sort(custom_compare);

    local getStr="";
    local authorizationString = "OAuth ";
    for ( local i=0; i< params.len(); i++ ) {
        if ( i!=0) {
            authorizationString += ",";
             getStr += "&";
        }
        getStr += params[i].a + "=" + url_encode(params[i].b);
    }

    local sign = "GET";
    sign += "&" + authStep1Url;

    return getStr;
}

function sendOauthRequest(method, url, params, token, tokenSecret) {
    local getStr = signRequest(method, url, params, token, tokenSecret);

    if ( method == "GET") {
        nm.doGet(url + "?" + getStr);
    } else if ( method == "POST") {
        nm.setUrl(url);
        local tableLen = params.len();

        for ( local i=0; i< tableLen; i++ ) {
            nm.addQueryParam( params[i].a, params[i].b);
        }
        nm.doPost("");
    }
    return 0;
}

function Authenticate() {
    oauth_token_secret = ServerParams.getParam("oauth_token_secret");
    oauth_token = ServerParams.getParam("oauth_token");

    if ( oauth_token_secret != ""  &&  oauth_token != ""){
        return 1;
    }
    local email = ServerParams.getParam("Login");

    local params = [{a="oauth_consumer_key", b=appKey},
                    {a="oauth_signature_method", b="HMAC-SHA1"},
                    {a="oauth_timestamp", b=""+time()},
                    {a="oauth_version", b="1.0"},
                    {a="oauth_nonce", b=generateNonce() },
                    {a="oauth_callback", b="oob"}
    ];
    sendOauthRequest("GET", authStep1Url, params, "", "");
    if (!checkResponse()) {
        return 0;
    }
    local data = nm.responseBody();

    local temp_oauth_token_secret = regex_simple(data, "oauth_token_secret=([^&]+)", 0);
    local temp_oauth_token = regex_simple(data, "oauth_token=([^&]+)", 0);
    if ( temp_oauth_token == "" ) {
        return 0;
    }

    ShellOpenUrl("https://www.flickr.com/services/oauth/authorize?oauth_token=" + temp_oauth_token);

    MessageBox(tr("flickr.confirmation.text", "Please allow Image Uploader to connect with your Flickr account (in web browser) and then press OK.\r\n\r\n"), "Flickr Uploader", "", "");

    local verificationCode = InputDialog(tr("flickr.verification", "Please enter verification code:"), "");
    local params2 = [
        {a="oauth_consumer_key", b=appKey},
        {a="oauth_signature_method", b="HMAC-SHA1"},
        {a="oauth_timestamp", b=""+time()},
        {a="oauth_version", b="1.0"},
        {a="oauth_nonce", b=generateNonce() },
        {a="oauth_verifier", b=verificationCode }
    ];
    sendOauthRequest("GET", authStep2Url, params2, temp_oauth_token, temp_oauth_token_secret);
    data =  nm.responseBody();

    oauth_token_secret = regex_simple(data, "oauth_token_secret=([^&]+)", 0);
    oauth_token = regex_simple(data, "oauth_token=([^&]+)", 0);

    if ( oauth_token_secret != "" && oauth_token != "" ) {
        ServerParams.setParam("oauth_token_secret", oauth_token_secret);
        ServerParams.setParam("oauth_token", oauth_token);
        return 1;
    }
    return 0;
}

function IsAuthenticated() {
    return ServerParams.getParam("oauth_token_secret") != "" && ServerParams.getParam("oauth_token") != "";
}

function UploadFile(FileName, options) {
    local oauth_token_secret = ServerParams.getParam("oauth_token_secret");
    local oauth_token = ServerParams.getParam("oauth_token");
    local userPath = ServerParams.getParam("UploadPath");
    if ( userPath!="" && userPath[userPath.len()-1] != "/") {
        userPath+= "/";
    }
    local path = "sandbox/" + userPath + ExtractFileName(FileName);

    local uploadUrl = "https://up.flickr.com/services/upload/";
    local albumId = options.getFolderID();

    nm.setUrl(uploadUrl);
    local params = [
        {a="oauth_consumer_key", b=appKey},
        {a="oauth_signature_method", b="HMAC-SHA1"},
        {a="oauth_timestamp", b=""+time()},
        {a="oauth_version", b="1.0"},
        {a="oauth_nonce", b=generateNonce() },
        {a="oauth_callback", b="oob"}
    ];

    signRequest("POST", uploadUrl, params, oauth_token, oauth_token_secret);
    nm.addQueryParamFile("photo", FileName, ExtractFileName(FileName), "");
    local tableLen = params.len();

    for ( local i=0; i< tableLen; i++ ) {
        nm.addQueryParam( params[i].a, params[i].b);
    }

    nm.doUploadMultipartData();
    local data = nm.responseBody();
    local photoid = regex_simple(data, "<photoid>(.+)</photoid>", 0);

    nm.doGet("https://api.flickr.com/services/rest/?method=flickr.photos.getSizes&format=json&nojsoncallback=1&api_key=" + appKey + "&photo_id="+photoid);

    if (nm.responseCode() != 200) {
        WriteLog("error", "[flickr] Failed to obtain links to images");
        return 0;
    }
    data = nm.responseBody();

    local t = ParseJSON(data);
    local originalPhoto = "";
    local thumbUrl = "";

    if (!("sizes" in t)) {
        WriteLog("error", "[flickr] Invalid response from server");
        return 0;
    }

    foreach (i, val in t.sizes.size) {
        if (val.label == "Original") {
            originalPhoto = val.source;
        } else if (val.label == "Thumbnail") {
            thumbUrl = val.source;
        }
    }

    if ( originalPhoto != "") {
        options.setDirectUrl(originalPhoto);
        options.setThumbUrl(thumbUrl);

        nm.doGet("https://api.flickr.com/services/rest/?method=flickr.photos.getInfo&format=json&nojsoncallback=1&api_key=" + appKey +"&photo_id=" + photoid +"&oauth_token="+oauth_token);

        local photoPage = "";
        if (nm.responseCode() == 200) {
            t = ParseJSON(nm.responseBody());
            if ("photo" in t) {
                foreach(i, val in t.photo.urls.url) {
                    if (val.type == "photopage") {
                        photoPage = val._content;
                        break;
                    }
                }
            }
        }

        options.setViewUrl(photoPage);

        if (albumId != "") {

            local params3=[	{a="oauth_consumer_key", b=appKey},
                {a="oauth_signature_method", b="HMAC-SHA1"},
                {a="oauth_timestamp", b=""+time()},
                {a="oauth_version", b="1.0"},
                {a="oauth_nonce", b=generateNonce() },
                {a="api_key", b=appKey},
                {a="method", b="flickr.photosets.addPhoto"},
                {a="photoset_id", b=albumId},
                {a="photo_id", b=photoid}
            ];

            sendOauthRequest("POST", "https://api.flickr.com/services/rest/", params3, oauth_token, oauth_token_secret);
        }
        return 1;
    }

    return 0;
}

function GetFolderList(list) {
    local oauth_token_secret = ServerParams.getParam("oauth_token_secret");
    local oauth_token = ServerParams.getParam("oauth_token");
    local params=[
        {a="oauth_consumer_key", b=appKey},
        {a="oauth_signature_method", b="HMAC-SHA1"},
        {a="oauth_timestamp", b=""+time()},
        {a="oauth_version", b="1.0"},
        {a="oauth_nonce", b=generateNonce() },
        {a="api_key", b=appKey},
        {a="format", b="json"},
        {a="nojsoncallback", b="1"},
        {a="method", b="flickr.photosets.getList"}
     ];

    sendOauthRequest("GET", "https://api.flickr.com/services/rest/", params, oauth_token, oauth_token_secret);
    if (nm.responseCode() != 200) {
        return 0;
    }

    local t = ParseJSON(nm.responseBody());
    if (!("photosets" in t)) {
        WriteLog("error", "[flickr] Invalid response from server");
        return 0;
    }

    foreach(i,item in t.photosets.photoset) {
        local album = CFolderItem();
        album.setId(item.id);
        album.setTitle(item.title._content);
        album.setSummary(item.description._content);
        list.AddFolderItem(album);
    }

    return 1; //success
}

/*
Flickr API requires a photo id for creating a photoset :(

function CreateFolder(parentAlbum, album) {
}
*/

function GetServerParamList() {
    return {
        oauth_token_secret = "oauth_token_secret"
        oauth_token = "oauth_token"
    }
}