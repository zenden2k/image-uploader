
#define TIXML_USE_STL
#include <tinyxml.h>
#include <vector>

class ZSimpleXmlNode
{private:
	TiXmlElement *m_el;
	public:
		ZSimpleXmlNode(TiXmlElement *el);
		ZSimpleXmlNode operator[](const std::string name);
		std::string Attribute(const std::string name) const;
		int AttributeInt(const std::string name) const;
		bool AttributeBool(const std::string name) const;
		std::string Name();
		std::string Text();

		// Write
		void SetAttribute(const std::string name, const std::string value);
		void SetAttribute(const std::string name, int value);
		void SetAttributeBool(const std::string name, bool value);
		bool IsNull();
		bool GetChilds(const std::string name,std::vector<ZSimpleXmlNode> &out);
		//size_t ChildCount(std::string name="");
};

typedef  std::vector<ZSimpleXmlNode>::iterator ZSimpleXmlNodeIterator;
class ZSimpleXml
{
	TiXmlDocument doc;
	TiXmlHandle *docHandle;
	public:
		ZSimpleXml();
		~ZSimpleXml();
		bool LoadFromFile(const std::string fileName);
		bool SaveToFile(const std::string fileName);
		ZSimpleXmlNode getRoot(const std::string name);
};
