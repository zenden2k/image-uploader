function regex_simple(data,regStr,start)
{
	local ex = regexp(regStr);
	local res = ex.capture(data, start);
	local resultStr = "";
	if(res != null){	
		resultStr = data.slice(res[1].begin, res[1].end);
	}
	return resultStr;
}

function  UploadFile(FileName, options)
{	
    local xml = SimpleXml();
    xml.LoadFromFile("d:\\Develop\\imageuploader-1.3.2-vs2013\\image-uploader\\Data\\servers.xml");
    local childs = xml.GetRoot("Servers", false).GetChildren("Server");
    foreach (i,el in childs) {
         print(el.Attribute("Name") + "\r\n");
    }
    return -1;
    local txt = "<h1><a id=\"logo\">some link</a></h1>";
    local doc = Document(txt);
    print(doc.find("h1 a").text()+"\r\n");
    print(doc.find("h1 a").length()+"\r\n");
    print(doc.find("#logo").attr("class")+"\r\n");
    return -1;
    nm.doGet("http://zenden.ws");
    txt = nm.responseBody();
    doc = Document(txt);
    doc.find("a").each( function(index,elem) {
        print(elem.text()+"-\r\n"); 
    });

    return -1;
    
    
	nm.setUrl("http://localhost/ImageUploader/Web/upload.php");
	nm.addQueryParamFile("file",FileName, ExtractFileName(FileName),"");
	local data = "";
	nm.doUploadMultipartData();
	data = nm.responseBody();

	local directUrl = regex_simple(data,"\\[url=(.+)\\]",0);
	local thumbUrl = regex_simple(data,"\\[img\\](.+)\\[/img\\]",0);

	options.setDirectUrl(directUrl);
	options.setThumbUrl(thumbUrl);
	
	if ( directUrl != "" ) {	
		return 1;
	} else {
		return 0;
	}
}