// Chevereto uploader
const CURLOPT_COOKIELIST = 10135;
const PUBLIC_API_KEY = "e9c24b3a3762e7e1721811833c218c8c458d2baa0aefa34f95e2ee2758e3f4f9";
function _PrintError(t, txt) {
    local errorMessage = "[imgtr.ee] " + txt + " Response code: " + nm.responseCode();
    if (t != null && "error" in t && t.error != null) {
        errorMessage += "\n" + t.error.message;
    }
    WriteLog("error", errorMessage);
}

function UploadFile(filePath, options) {
    local login =  ServerParams.getParam("Login");
    local apiKey = ServerParams.getParam("Password");
    if (apiKey == "") {
        if (login != "") {
            WriteLog("error", "[imgtr.ee] " + tr("chevereto.api_key_not_set"));
            return ResultCode.Failure;
        }
        apiKey = PUBLIC_API_KEY; 
    }

    local task = options.getTask().getFileTask();
    nm.setUrl("https://imgtr.ee/api/1/upload");
    nm.addQueryHeader("X-API-Key", apiKey);
    nm.addQueryParamFile("source", filePath, task.getDisplayName(), GetFileMimeType(filePath));
    nm.doUploadMultipartData();

    local t = ParseJSON(nm.responseBody());
    if (nm.responseCode() != 200) {
        _PrintError(t, "Upload failed.");
        return ResultCode.Failure;
    }

    if (t != null) {
        if ("status_code" in t && t.status_code == 200 && "image" in t) {
            options.setViewUrl(t.image.url_viewer);
            options.setThumbUrl(t.image.thumb.url);
            options.setDirectUrl(t.image.url);
            options.setDeleteUrl(t.image.delete_url);
            return ResultCode.Success;
        } else {
            _PrintError(t, "Upload failed.");
        }
    } else {
        _PrintError(t, "Failed to parse server response.");
    }

    return ResultCode.Failure;
}