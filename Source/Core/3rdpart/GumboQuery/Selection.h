#ifndef SELECTION_H_
#define SELECTION_H_

#include "Object.h"
#include <vector>
#include <string>
#include <functional>
#include <gumbo.h>
#include "Core/Scripting/Squirrelnc.h"

class CNode;

/**
 * @brief (gumbo-query) Selection returned by Document's find function()
 * 
 */
class CSelection: public CObject
{

	public:
        CSelection();
        /* @cond PRIVATE */
		CSelection(GumboNode* apNode);

		CSelection(std::vector<GumboNode*> aNodes);
        /* @endcond */
        std::string attribute(std::string key);

        std::string text();
        std::string tagName();
        std::string ownText();


		virtual ~CSelection();
        /* @cond PRIVATE */
        CSelection& each(std::function<void(int, CNode)> callback);
        /* @endcond */
        CSelection& squirrelEach(Sqrat::Function callback);


		CSelection find(std::string aSelector);

        CNode at(unsigned int i);
        /* @cond PRIVATE */
		CNode nodeAt(unsigned int i);
        /* @endcond */

		unsigned int nodeNum();

	private:

		std::vector<GumboNode*> mNodes;
};

#endif /* SELECTION_H_ */

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
