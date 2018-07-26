function getToken(data) {
    local ex = regexp("name=\"authenticity_token\"?.+(value=\"(.+)\")");
    local res = ex.capture(data, 0);
    local resultStr = "";

    if (res != null) {
        resultStr = data.slice(res[2].begin, res[2].end);
    }
    return resultStr;
}

function UploadFile(FileName, options) {
    local file_name = ExtractFileName(FileName);
    local file_uid = "";
    local file_url = "";
    local file_token = "";
    local file_uploadServer = "http://muonium.rgho.st/files";
    nm.doGet("http://rgho.st/");
    if (nm.responseCode() == 200) {
        local serverData = nm.responseBody();
        file_token = getToken(serverData);

        nm.doGet("http://rgho.st/api/blank?filename=" + nm.urlEncode(file_name));
        if (nm.responseCode() == 200) {
            local serverData = nm.responseBody();
            local js = ParseJSON(serverData);
            file_uid = js.uid;
            file_url = js.url;

            //upload
            nm.setUrl(file_uploadServer);
            nm.addQueryParam("utf8", "&#x2713;");
            nm.addQueryParam("authenticity_token", file_token);
            nm.addQueryParam("uuid", file_uid);
            nm.addQueryParamFile("file", FileName, file_name, "");
            nm.addQueryParam("commit", "Отправить");
            nm.doUploadMultipartData();
            options.setViewUrl(file_url);
            return 1;
        } else {
            return 0;
        }

    } else {
        return 0;
    }
}