nm.setMethod("PUT");
nm.setUrl("https://www.googleapis.com/drive/v2/files/" + id);
nm.addQueryHeader("Authorization", "Basic ");
nm.addQueryHeader("Content-Type", "application/json");
local postData = {title= "SmellyCat.jpg"};
nm.doUpload("", ToJSON(postData));