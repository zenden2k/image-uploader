#ifndef CORE_UPLOAD_FILEUPLOADTASK_H
#define CORE_UPLOAD_FILEUPLOADTASK_H

#include "FolderList.h"
#include <string>

void CFolderList::AddFolder(const std::string& title, const std::string& summary, const std::string& id,
                            const std::string& parentid,
                            int accessType)
{
    CFolderItem ai;
    ai.title = (title);
    ai.summary = (summary);
    ai.id = (id);
    ai.parentid =  (parentid);
    ai.accessType = accessType;
    m_folderItems.push_back(ai);
}

void CFolderList::AddFolderItem(const CFolderItem& item)
{
    m_folderItems.push_back(item);
}


#endif