local t = ParseJSON(nm.responseBody());
if (t!= null && "thumb_url" in t) {
    WriteLog("info", t.thumb_url);
}