local xml = SimpleXml();
xml.LoadFromFile("servers.xml");
xml.GetRoot("Servers", false).each(function(i,el) { 
    if ( el.Name() == "Server" ) {
        print(el.Attribute("Name") + "\r\n");
    }
});