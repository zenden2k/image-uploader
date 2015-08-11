local xml = SimpleXml();
xml.LoadFromFile("d:\\Develop\\imageuploader-1.3.2-vs2013\\image-uploader\\Data\\servers.xml");
local childs = xml.GetRoot("Servers", false).GetChildren("Server");
foreach (i,el in childs) {
	print(el.Attribute("Name") + "\r\n");
}