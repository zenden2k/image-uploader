local fileName = "c:\\test\\file.txt";
nc.setUrl("ftp://example.com");
nc.setMethod("PUT");
nc.doUpload(fileName, "");