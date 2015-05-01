/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#ifndef IU_BASE_H
#define IU_BASE_H

#include <map>
#include <vector>
#include "HistoryManager.h"

class ZBase
{
    public:
        ZBase();
        static ZBase* get();
        CHistoryManager* historyManager();
        void addToGlobalCache(const std::string fileName, const std::string url);
        std::string getFromCache(const std::string url);

    private:
        static ZBase* m_base;
        CHistoryManager m_hm;
        std::map<std::string, std::string> m_UrlCache;
        static void cleanUp();
};
#endif
