#ifndef CORE_UPLOAD_FOLDERLIST_H
#define CORE_UPLOAD_FOLDERLIST_H
#pragma once

#include "UploadEngine.h"
#include <string>
#include <vector>

/**
CFolderList class
*/
class CFolderList
{
    public:
        /*! @cond PRIVATE */
        std::vector<CFolderItem> m_folderItems;
        /*! @endcond */

        /**
        @since 1.3.2
        */
        int GetCount() { return m_folderItems.size();}
        /*! @cond PRIVATE */
        CFolderItem& operator [] (int index) {  return m_folderItems[index]; }
        void Clear() { m_folderItems.clear(); }
        /*! @endcond */
        void AddFolder(const std::string& title, const std::string& summary, const std::string& id, const std::string& parentid, int accessType);
        void AddFolderItem(const CFolderItem& item);
};

#endif