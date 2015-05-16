local txt = "<h1><a id=\"logo\">some link</a></h1>";
local doc = Document(txt);
print(doc.find("h1 a").text()+"\r\n");
print(doc.find("h1 a").length()+"\r\n");
print(doc.find("#logo").attr("class")+"\r\n");
    
nm.doGet("http://zenden.ws");
txt = nm.responseBody();
doc = Document(txt);
doc.find("a").each( function(index,elem) {
    print(elem.text()+"-\r\n"); 
});