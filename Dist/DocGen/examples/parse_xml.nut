function _GetUploadUrl() {
    local xml = SimpleXml();
    xml.LoadFromString(nm.responseBody());
    local root = xml.GetRoot("response", false);
        
    if (root.IsNull()) {
        WriteLog("error", "[example.com]: Invalid response when getting upload URL");
        return null;
    }
        
    local statusNode = root.GetChild("status", false);
    if (statusNode.IsNull() || statusNode.Text() != "OK") {
        WriteLog("error", "[example.com]: Error status when getting upload URL");
        return null;
    }
        
    local urlNode = root.GetChild("url", false);
    if (urlNode.IsNull()) {
        WriteLog("error", "[example.com]: Upload URL not found in response");
        return null;
    }
        
    return urlNode.Text();
}

