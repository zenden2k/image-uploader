//expect_streq("en", GetAppLanguage());
//expect_streq("en_US", GetAppLocale());

expect_eq("QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciBhIG51bWJlciBvZiBzaW1pbGFyIGVuY29kaW5nIHNjaGVtZXMgdGhhdCBlbmNvZGU=", Base64Encode("Base64 is a generic term for a number of similar encoding schemes that encode"));
expect_eq("Base64 is a generic term for a number of similar encoding schemes that encode", Base64Decode("QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciBhIG51bWJlciBvZiBzaW1pbGFyIGVuY29kaW5nIHNjaGVtZXMgdGhhdCBlbmNvZGU="));
    
local fileName = "c:\\Program Files (x86)\\Image Uploader\\Image Uploader.exe";
expect_eq("Image Uploader.exe", ExtractFileName(fileName));

local t = {
    a = "test1",
    b = 1.0,
    c = 256,
    d = null
};
expect_eq("{\n   \"a\" : \"test1\",\n   \"b\" : 1.0,\n   \"c\" : 256,\n   \"d\" : null\n}", ToJSON(t));

local t2 = {
    a = {
        aa = "test"
    },
    b = [1,2,3]
};
local expected = "{\n   \"a\" : \n   {\n      \"aa\" : \"test\"\n   },\n   \"b\" : [ 1, 2, 3 ]\n}";
local actual = ToJSON(t2);
//expect_eq(expected.len(), actual.len());
expect_streq(expected, actual);


function ParseJSONTest() {
    // you can put a double quotation mark inside a verbatim string by doubling it
    local jsonStr = @"
    {
        ""employees"":[
        {""firstName"":""John"", ""lastName"":""Doe""},
        {""firstName"":""Anna"", ""lastName"":""Smith""},
        {""firstName"":""Peter"", ""lastName"":""Jones""}
        ]
    }
    ";

    local tbl = ParseJSON(jsonStr);
    expect_streq("Smith", tbl.employees[1].lastName);
    expect_streq("John", tbl.employees[0].firstName);
    expect_eq(3, tbl.employees.len());

    // Primitive types
    expect_eq(null, ParseJSON("null"));
    expect_eq(123, ParseJSON("123"));
    expect_streq("Anna Smith", ParseJSON("\"Anna Smith\""));
    expect_eq(true, ParseJSON("true"));
    expect_eq(false, ParseJSON("false"));
    expect_eq(0.25, ParseJSON("0.25"));

    // Errors
    expect_eq(null, ParseJSON("&-----<>"));
    expect_eq(null, ParseJSON(""));
}

ParseJSONTest();