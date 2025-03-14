const BASE_HOST = "https://imageup.ru";

function Authenticate() { 
    local login = ServerParams.getParam("Login");
    local password = ServerParams.getParam("Password");

    if(login == "" ) {
        WriteLog("error", "Login should not be empty!");
        return 0;
    }
    
    nm.addQueryParam("login",login);
    nm.addQueryParam("password", password);
    nm.addQueryParam("autologin", "on");
    nm.setUrl(BASE_HOST + "/login.html");
    nm.doPost("");
    
    if (nm.responseCode() == 200) {
        if (nm.responseBody().find("Добро пожаловать на сайт") != null) {
            return 1;
        }
    }
    WriteLog("error", "[imageup.ru] authentication failed.");
    
    return 0;        
} 

function UploadFile(fileName, options) {
    nm.doGet(BASE_HOST + "/");
    local doc = Document(nm.responseBody());
    local maxFileSize = doc.find("input[name=MAX_FILE_SIZE]").attr("value");
    local fileFieldName = doc.find("input[type=file]").attr("name");
    if (fileFieldName == "") {
        fileFieldName = "file2022";
    }
    local timestamp = 0, token = "";
    local captchaAnswer = "";
    local reg = CRegExp("token'\\s*:\\s*'(.+?)'", "mi"); 
    if (reg.match(nm.responseBody()) ) {
        token = reg.getMatch(1);
    }
    local reg2 = CRegExp("timestamp'\\s*:\\s*'(.+?)'", "mi"); 
    if ( reg2.match(nm.responseBody()) ) {
        timestamp = reg2.getMatch(1);
    }

    local captchaUrl = doc.find("#captcha").attr("src");
    if (captchaUrl != "") {
        if (captchaUrl.find("https://") == null) {
            captchaUrl = BASE_HOST + captchaUrl;
        }
        captchaAnswer = AskUserCaptcha(nm, captchaUrl);
        if (captchaAnswer == "") {
            return -1;
        }
    }

    nm.setUrl(BASE_HOST + "/");
    nm.addQueryParam("MAX_FILE_SIZE", maxFileSize);
    nm.addQueryParamFile(fileFieldName, fileName, ExtractFileName(fileName), GetFileMimeType(fileName));
    if (captchaAnswer != "") {
        nm.addQueryParam("captchatext", captchaAnswer);
    }
    
    nm.addQueryParam("makepreview", "on");
    nm.addQueryParam("previewsize", "300");
    nm.addQueryParam("textonpreview", "on");
    nm.addQueryParam("textonpreview_opt", "1");
    nm.addQueryParam("textonpreview_inp", "");
    nm.addQueryParam("radiobox", "1");
    nm.addQueryParam("usertext", "");
    nm.addQueryParam("resize", "500");
    nm.addQueryParam("rotate", "0");
    
    nm.doUploadMultipartData();
    
    if (nm.responseCode() == 200) {
        local doc2 = Document(nm.responseBody());
        local viewUrl = doc2.find("#id8").attr("value");
        local directUrl = doc2.find("#id7").attr("value");
        local deleteUrl = doc2.find("#id_delete").attr("value");
        local previewBbCode = doc2.find("#id3").attr("value");
        local reg = CRegExp("\\[img\\](.+?)\\[/img\\]", "im");
        local thumbUrl = "";

        if (viewUrl != "" && viewUrl.find("https://") != 0) {
            viewUrl = "https://" + viewUrl;
        }

        options.setDirectUrl(directUrl);
        options.setViewUrl(viewUrl);
        options.setDeleteUrl(deleteUrl);

        if (reg.match(previewBbCode) ) {
            options.setThumbUrl(reg.getMatch(1));
        }

        if (directUrl != "" || viewUrl != ""){
            return 1;
        } else {
            WriteLog("error", "[imageup.ru] Failed to obtain image link");
        }
    } else {
        WriteLog("error", "[imageup.ru] Failed to upload, response code = " + nm.responseCode());
    }

    return 0;
}