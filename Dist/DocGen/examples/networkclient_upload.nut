local fileName = "c:\\test\\file.txt"; //only UTF-8 file names are supported on Windows
nm.setUrl("http://takebin.com/action");
nm.addQueryParamFile("file", fileName, ExtractFileName(FileName),"");
nm.addQueryParam("fileDesc", "cool file");

nm.doUploadMultipartData();
if ( nm.responseCode() == 200 )  {
	print(nm.responseBody());
}