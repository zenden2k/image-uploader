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

#include "PluginLoader.h"
#include <io.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <sqstdsystem.h>

#include "atlheaders.h"
#include "Core/Network/NetworkClient.h"
#include "Func/Settings.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Core/Upload/ScriptUploadEngine.h"

CPluginManager::CPluginManager() {
	/*SquirrelVM::Init();
	SquirrelVM::PushRootTable();	*/
	CScriptUploadEngine::InitScriptEngine();
}

CScriptUploadEngine* CPluginManager::getPlugin(const Utf8String& serverName, const Utf8String& name,  ServerSettingsStruct& params, bool UseExisting) {
	DWORD curTime = GetTickCount();
	CScriptUploadEngine* plugin = m_plugins[serverName];
	if (plugin && (GetTickCount() - plugin->getCreationTime() < 1000 * 60 * 5))
		UseExisting = true;

	if(plugin && UseExisting && plugin->name() == name && plugin->serverSettings().authData.Login ==  params.authData.Login ) {
		plugin->onErrorMessage.bind( DefaultErrorHandling::ErrorMessage );
		return plugin;
	}

	if (plugin) {	
		delete plugin; 
		plugin = 0;
		m_plugins[serverName]  = 0;
	}

	CScriptUploadEngine* newPlugin = new CScriptUploadEngine( name );
	newPlugin->onErrorMessage.bind( DefaultErrorHandling::ErrorMessage );
	if (newPlugin->load( m_ScriptsDirectory + name + ".nut", params ) ) {
		m_plugins[serverName] = newPlugin; 
		return newPlugin;
	} else {
		delete newPlugin;
	}
	return NULL;
}

CPluginManager::~CPluginManager() {
	UnloadPlugins();
}

void CPluginManager::UnloadPlugins() {
	std::map<Utf8String,CScriptUploadEngine*>::iterator it;
	for(it = m_plugins.begin(); it!= m_plugins.end(); ++it) {
		delete it->second;
	}
	m_plugins.clear();
}

void CPluginManager::setScriptsDirectory(const Utf8String & directory) {
	m_ScriptsDirectory = directory;
}