/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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