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

#include "Selection.h"
#include "Parser.h"
#include "QueryUtil.h"
#include "Node.h"

CSelection::CSelection() {
}

CSelection::CSelection(GumboNode* apNode)
{
	mNodes.push_back(apNode);
}

CSelection::CSelection(std::vector<GumboNode*> aNodes)
{
	mNodes = aNodes;
}

std::string CSelection::attribute(std::string key) {
    return nodeAt(0).attribute(key);
}

std::string CSelection::text() {
    std::string res;
    for (std::vector<GumboNode*>::iterator it = mNodes.begin(); it != mNodes.end(); it++) {
        GumboNode* pNode = *it;
        res += CNode(pNode).text();
    }
    return res;
}

std::string CSelection::tagName() {
    return nodeAt(0).tag();
}

std::string CSelection::ownText() {
    std::string res;
    for (std::vector<GumboNode*>::iterator it = mNodes.begin(); it != mNodes.end(); it++) {
        GumboNode* pNode = *it;
        res += CNode(pNode).ownText();
    }
    return res;
}

CSelection::~CSelection()
{
}

CSelection& CSelection::each(std::function<void(int, CNode)> callback) {
    int i = 0;
    for (std::vector<GumboNode*>::iterator it = mNodes.begin(); it != mNodes.end(); it++) {
        GumboNode* pNode = *it;
        callback(i, CNode(pNode));
        i++;
    }
    return *this;
}

CSelection& CSelection::squirrelEach(Sqrat::Function callback) {
    int i = 0;
    for (std::vector<GumboNode*>::iterator it = mNodes.begin(); it != mNodes.end(); it++) {
        GumboNode* pNode = *it;
        Sqrat::SharedPtr<bool> res = callback.Evaluate<bool>(i, CNode(pNode));
        if (!!res && *res) {
            break;
        }
        i++;
    }
    return *this;
}

CSelection CSelection::find(std::string aSelector)
{
	CSelector* sel = CParser::create(aSelector);
	std::vector<GumboNode*> ret;
	for (std::vector<GumboNode*>::iterator it = mNodes.begin(); it != mNodes.end(); it++)
	{
		GumboNode* pNode = *it;
		std::vector<GumboNode*> matched = sel->matchAll(pNode);
		ret = CQueryUtil::unionNodes(ret, matched);
	}
	sel->release();
	return CSelection(ret);
}

CNode CSelection::nodeAt(unsigned int i)
{
	if (i >= mNodes.size())
	{
		return CNode();
	}

	return CNode(mNodes[i]);
}

unsigned int CSelection::length()
{
	return mNodes.size();
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */

