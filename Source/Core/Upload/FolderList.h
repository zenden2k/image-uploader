#ifndef CORE_UPLOAD_FOLDERLIST_H
#define CORE_UPLOAD_FOLDERLIST_H
#pragma once

#include "UploadEngine.h"
#include <string>
#include <vector>

class CFolderList
{
    public:
        std::vector<CFolderItem> m_folderItems;
        void Clear() {m_folderItems.clear();}
        const int GetCount() { return m_folderItems.size();}
        CFolderItem& operator [] (int index) {  return m_folderItems[index]; }
        void AddFolder(const std::string& title, const std::string& summary, const std::string& id, const std::string& parentid, int accessType);
        void AddFolderItem(const CFolderItem& item);
};

#endif