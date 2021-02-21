/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#ifndef IU_CORE_UPLOADENGINELIST_H
#define IU_CORE_UPLOADENGINELIST_H

#pragma once

#include <string>
#include "Upload/UploadEngine.h"
#include "Core/Utils/CoreTypes.h"

class CUploadEngineList: public CUploadEngineListBase
{
    public:
        CUploadEngineList();
        bool loadFromFile(const std::string& filename, ServerSettingsMap&);
        void setNumOfRetries(int Engine, int Action);
        bool addServer(const CUploadEngineData& data);
    protected:
        int m_EngineNumOfRetries;
        int m_ActionNumOfRetries;
    private:
        DISALLOW_COPY_AND_ASSIGN(CUploadEngineList);
        bool static compareEngines(const CUploadEngineData& elem1, const CUploadEngineData& elem2);
};

#endif // IU_CORE_UPLOADENGINELIST_H