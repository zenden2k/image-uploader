#include <gtest/gtest.h>

#include <boost/format.hpp>

#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/SimpleXml.h"
#include "Tests/TestHelpers.h"

class SimpleXmlTest : public ::testing::Test {
public:
    SimpleXmlTest()
        : fileName_(TestHelpers::resolvePath("servers.xml"))
    {
    }

protected:
    const std::string fileName_;
};

TEST_F(SimpleXmlTest, BasicRead) {
    {
        const char* xml = "<?xml version=\"1.0\"?>"
                          "<PLAY>"
                          "<TITLE>A Midsummer Night's Dream</TITLE>"
                          "</PLAY>";

        SimpleXml doc;
        ASSERT_EQ(true, doc.LoadFromString(xml));
        auto node = doc.getRoot("PLAY", false).GetChild("TITLE", false);
        EXPECT_TRUE(!node.IsNull());
        EXPECT_EQ("A Midsummer Night's Dream", node.Text());
    }
    {
        static const char* xml = "<information>"
                                 "	<attributeApproach v='2' />"
                                 "	<textApproach>"
                                 "		<v>2</v>"
                                 "	</textApproach>"
                                 "</information>";
        SimpleXml doc;
        ASSERT_EQ(true, doc.LoadFromString(xml));
        auto root = doc.getRoot("information", false);
        EXPECT_TRUE(!root.IsNull());
        EXPECT_EQ(2, root.GetChild("attributeApproach", false).AttributeInt("v"));
        auto v2Node = root.GetChild("textApproach", false).GetChild("v", false).Text();
        EXPECT_EQ("2", v2Node);
    }
}

TEST_F(SimpleXmlTest, Children) {
    const char* xml = "<?xml version ='1.0'?>"
                      "<!-- Top level comment. -->"
                      "<root>"
                      "    <child foo='bar'/>"
                      "    <child foo='222'/>"
                      "    <!-- comment thing -->"
                      "    <child2 val='1'>Text</child2>"
                      "    <child foo=\"empty\"/>"
                      "</root>";
    SimpleXml doc;
    ASSERT_EQ(true, doc.LoadFromString(xml));
    auto root = doc.getRoot("root", false);
    EXPECT_TRUE(!root.IsNull());
    EXPECT_EQ(4, root.GetChildCount());
    auto childNo2 = root.GetChildByIndex(2);
    EXPECT_EQ("child2", childNo2.Name());

    {
        std::vector<SimpleXmlNode> children;
        EXPECT_TRUE(root.GetChilds("child", children));
        ASSERT_EQ(3, children.size());
        EXPECT_EQ("bar", children[0].Attribute("foo"));
        EXPECT_EQ(222, children[1].AttributeInt("foo"));
        EXPECT_EQ("empty", children[2].Attribute("foo"));
    }
    {
        std::vector<SimpleXmlNode> children2;
        EXPECT_TRUE(root.GetChilds("child2", children2));
        ASSERT_EQ(1, children2.size());
        EXPECT_EQ(1, children2[0].AttributeInt("val"));
        EXPECT_EQ("Text", children2[0].Text());
    }
    {
        std::vector<SimpleXmlNode> children3;
        root.each([&](int, SimpleXmlNode& child) -> bool {
            children3.push_back(child);
            return false;
        });
        ASSERT_EQ(4, children3.size());
        EXPECT_EQ(1, children3[2].AttributeInt("val"));
    }

    root.DeleteChilds();
    EXPECT_EQ(0, root.GetChildCount());
}

TEST_F(SimpleXmlTest, LoadFromFile) {
    SimpleXml doc;
    ASSERT_EQ(true, doc.LoadFromFile(fileName_));
    auto node = doc.getRoot("Servers", false);
    EXPECT_TRUE(!node.IsNull());
    EXPECT_EQ("Servers", node.Name());
    EXPECT_GT(node.GetChildCount(), 0);
    auto serverNode = node.GetChild("Server", false);
    EXPECT_EQ("radikal.ru", serverNode.Attribute("Name"));
}

TEST_F(SimpleXmlTest, Attributes) {
    const char* xml = "<?xml version ='1.0'?>"
                      "<!-- Top level comment. -->"
                      "<root>"
                      "    <child foo='bar' val='123' enable=\"1\" bigint='9223372036854775807' />"
                      "</root>";

    SimpleXml doc;
    ASSERT_EQ(true, doc.LoadFromString(xml));
    auto node = doc.getRoot("root", false).GetChild("child", false);
    EXPECT_TRUE(!node.IsNull());
    EXPECT_EQ(4, node.GetAttributeCount());
    EXPECT_EQ(123, node.AttributeInt("val"));
    EXPECT_TRUE(node.AttributeBool("enable"));
    EXPECT_EQ("bar", node.Attribute("foo"));
    EXPECT_EQ(9223372036854775807, node.AttributeInt64("bigint"));

    {
        int val = 0;
        EXPECT_TRUE(node.GetAttributeInt("val", val));
        EXPECT_EQ(123, val);
    }
    {
        std::string val;
        EXPECT_TRUE(node.GetAttribute("foo", val));
        EXPECT_EQ("bar", val);
    }
    {
        bool val = false;
        EXPECT_TRUE(node.GetAttributeBool("enable", val));
        EXPECT_TRUE(val);
    }

    std::vector<std::string> attributes;
    node.GetAttributes(attributes);
    EXPECT_GE(attributes.size(), 4);
    EXPECT_EQ("foo", attributes[0]);
    EXPECT_EQ("val", attributes[1]);
    EXPECT_EQ("enable", attributes[2]);
    EXPECT_EQ("bigint", attributes[3]);
}
