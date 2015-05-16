/***************************************************************************
 * 
 * $Id$
 * 
 **************************************************************************/

/**
 * @file $HeadURL$
 * @author $Author$(hoping@baimashi.com)
 * @date $Date$
 * @version $Revision$
 * @brief 
 *  
 **/

#include "Node.h"
#include "Selection.h"
#include "QueryUtil.h"

CNode::CNode() {
    mpNode = NULL;
}

CNode::CNode(GumboNode* apNode)
{
	mpNode = apNode;
}

CNode::~CNode()
{
}

CNode CNode::parent()
{
    if ( mpNode == NULL) {
        return CNode();
    }
	return CNode(mpNode->parent);
}

CNode CNode::nextSibling()
{
    if (mpNode == NULL) {
        return CNode();
    }
	return parent().childAt(mpNode->index_within_parent + 1);
}

CNode CNode::prevSibling()
{
    if (mpNode == NULL) {
        return CNode();
    }
	return parent().childAt(mpNode->index_within_parent - 1);
}

unsigned int CNode::childNum()
{
    if (mpNode == NULL) {
        return 0;
    }
	if (mpNode->type != GUMBO_NODE_ELEMENT)
	{
		return 0;
	}

	return mpNode->v.element.children.length;

}

bool CNode::valid()
{
	return mpNode != NULL;
}

CNode CNode::childAt(unsigned int i)
{
    if (mpNode == NULL) {
        return CNode();
    }
	if (mpNode->type != GUMBO_NODE_ELEMENT || i >= mpNode->v.element.children.length)
	{
		return CNode();
	}

	return CNode((GumboNode*) mpNode->v.element.children.data[i]);
}

std::string CNode::attribute(std::string key)
{
    if (mpNode == NULL) {
        return "";
    }
	if (mpNode->type != GUMBO_NODE_ELEMENT)
	{
		return "";
	}

	GumboVector attributes = mpNode->v.element.attributes;
	for (unsigned int i = 0; i < attributes.length; i++)
	{
		GumboAttribute* attr = (GumboAttribute*) attributes.data[i];
		if (key == attr->name)
		{
			return attr->value;
		}
	}

	return "";
}

std::string CNode::text()
{
    if (mpNode == NULL) {
        return std::string();
    }
	return CQueryUtil::nodeText(mpNode);
}

std::string CNode::ownText()
{
    if (mpNode == NULL) {
        return std::string();
    }
	return CQueryUtil::nodeOwnText(mpNode);
}

std::string CNode::tag()
{
    if (mpNode == NULL) {
        return std::string();
    }
	if (mpNode->type != GUMBO_NODE_ELEMENT)
	{
		return "";
	}

	return gumbo_normalized_tagname(mpNode->v.element.tag);
}

CSelection CNode::find(std::string aSelector)
{
	CSelection c(mpNode);
	return c.find(aSelector);
}
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
