local fileName = "c:\\test\\file.txt"; //only UTF-8 file names are supported on Windows
nm.setUrl("http://takebin.com/action");
nm.addPostFieldFile("file", fileName, ExtractFileName(FileName),GetFileMimeType(fileName));
nm.addPostField("fileDesc", "cool file");

nm.doUploadMultipartData();
if (nm.responseCode() == 200)  {
	print(nm.responseBody());
}