local xml = SimpleXml();
xml.LoadFromFile("servers.xml");
local childs = xml.GetRoot("Servers", false).GetChildren("Server");
foreach (i,el in childs) {
	print(el.Attribute("Name") + "\r\n");
}