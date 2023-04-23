#include <gtest/gtest.h>
#include "../Document.h"
#include "../Node.h"
#include <gumbo.h>
#include "Core/Utils/CoreUtils.h"
#include "Tests/TestHelpers.h"

class GumboTest : public ::testing::Test {

};
/*
TEST_F(GumboTest, Simple)
{
    std::string page("<h1><a>some link</a></h1>");
    CDocument doc;
    doc.parse(page.c_str());
     
    CSelection c = doc.find("h1 a");
    EXPECT_EQ("some link", c.nodeAt(0).text());
    //std::cout << c.nodeAt(0).attribute("class") << std::endl; // some link
}*/

TEST_F(GumboTest, Simple2)
{
    std::string page;
    //std::string page("<h1><a id=\"logo\">some link</a></h1>");
    bool readResult = IuCoreUtils::ReadUtf8TextFile(TestHelpers::resolvePath("test.html"), page);
    //EXPECT_EQ(true, readResult);
    CDocument doc;
    doc.parse(page);
    CSelection c = doc.find("#logo");
    if (c.nodeAt(0).valid()) {
        
        std::cout << c.nodeAt(0).attribute("class") << std::endl; // some link
    }
   

   // std::cout << c.nodeAt(0).attribute("class") << std::endl; // some link
}
/*
TEST_F(GumboTest, Simple2)
{
    GumboOutput* output = gumbo_parse("<h1>Hello, World!</h1>");
    std::cout<<output->root->v.text.text;
    // Do stuff with output->root
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}
*/

