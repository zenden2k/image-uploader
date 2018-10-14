expect_streq("en", GetAppLanguage());
expect_streq("en_US", GetAppLocale());

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
expect_eq("{\n   \"a\" : \"test1\",\n   \"b\" : 1,\n   \"c\" : 256,\n   \"d\" : null\n}", ToJSON(t));

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
