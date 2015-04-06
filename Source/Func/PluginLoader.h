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
#ifndef IU_FUNC_PLUGINLOADER_H
#define IU_FUNC_PLUGINLOADER_H
#pragma once

#include "atlheaders.h"
#include <windows.h>
#include "Core/Network/NetworkClient.h"
#include "Core/Upload/ScriptUploadEngine.h"
#include "Core/Utils/CoreUtils.h"
#include "Func/MyUtils.h"
#include "Func/Common.h"
#include <map>

class CPluginManager
{
	public:
		CPluginManager();
		~CPluginManager();
		void UnloadPlugins();
		void setScriptsDirectory(const Utf8String & directory);
		CScriptUploadEngine* getPlugin(const Utf8String& serverName, const Utf8String& name, ServerSettingsStruct& params, bool UseExisting = false);
	protected:
		std::map<Utf8String,CScriptUploadEngine*> m_plugins;
		Utf8String m_ScriptsDirectory;
	private:
		DISALLOW_COPY_AND_ASSIGN(CPluginManager);
};

#endif // IU_FUNC_PLUGINLOADER_H