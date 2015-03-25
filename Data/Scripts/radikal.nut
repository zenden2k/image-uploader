include("Utils/RegExp.nut");

function auth()
{
	local login = ServerParams.getParam("Login");
	local pass =  ServerParams.getParam("Password");
	if(login!="" && pass != "")
	{
		nm.setUrl("http://radikal.ru/Auth/Login");
		nm.addQueryParam("Login", login);
		nm.addQueryParam("Password", pass);
		nm.addQueryParam("IsRemember", "true");
		nm.addQueryParam("ReturnUrl", "/");
	}
	return true;
}

function  UploadFile(FileName, options)
{
	if(!auth()) return false;
	local thumbwidth = 180;
	try //for compatibility with IU versions < 1.2.7
	{
	  thumbwidth = options.getParam("THUMBWIDTH");
	}
	catch(ex){}
	
	nm.setUrl("http://radikal.ru/api3/rest/uplimg/prepared");
	nm.addQueryHeader("Referer","http://radikal.ru/Content/Clients/FotoFlashApplet3.swf?v=13");
	nm.addQueryHeader("User-Agent","Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/36.0.1985.125 Safari/537.36");
	
	nm.addQueryParam("RESPTYPE", "xml");
	nm.addQueryParam("FROMOBJECT", "22");
	nm.addQueryParam("MAXSIZEIMG", "1024");
	nm.addQueryParam("MAXSIZEIMGPREV", thumbwidth);
	nm.addQueryParam("ISINSCRIMGPREV", "true");
	nm.addQueryParam("INSCRIMGPREV", "Увеличить");
	nm.addQueryParam("IDALBUM", "noall");
	nm.addQueryParam("COMMENTIMG", "");
	nm.addQueryParam("URLFORIMG", "");
	nm.addQueryParam("TAGS", "");
	nm.addQueryParam("ISPUBLIC", "false");
		
	nm.addQueryParam("Filename", ExtractFileName(FileName));
	nm.addQueryParamFile("Filedata",FileName, ExtractFileName(FileName),GetFileMimeType(FileName));

		nm.addQueryParam("Upload", "Submit Query");
	local data = "";
	nm.doUploadMultipartData();
	data = nm.responseBody();
	if(data == "")
	{
		print ("Empty response");
		return 0;
	}
	local directUrl = regex_simple(data, "<ImgUrl>(.+)<", 0);
	local thumbUrl = regex_simple(data, "<ImgPreviewUrl>(.+)<", 0);
	//local viewUrl = regex_simple(data, "url\":\"(.+)\"", 0);

	options.setDirectUrl(directUrl);
	//options.setViewUrl(viewUrl);
	options.setThumbUrl(thumbUrl);	
	return 1;
}