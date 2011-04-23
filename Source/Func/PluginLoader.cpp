/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../atlheaders.h"
#include "PluginLoader.h"
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include "../Core/Network/NetworkManager.h"
#include <io.h>
#include "Settings.h"
#include "../Gui/Dialogs/LogWindow.h"

CPluginManager:: CPluginManager()
{
	SquirrelVM::Init();
	SquirrelVM::PushRootTable();	
	sqstd_register_systemlib(SquirrelVM::GetVMPtr());
}

CScriptUploadEngine* CPluginManager::getPlugin(Utf8String name, ServerSettingsStruct& params, bool UseExisting)
{
	DWORD curTime = GetTickCount();
	if(m_plugin && (GetTickCount() - m_plugin->getCreationTime()<1000*60*5))
		UseExisting = true;

	if(m_plugin && UseExisting && m_plugin->name() == name)
	{
		m_plugin->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
		return m_plugin;
	}

	if(m_plugin)
	{	
		delete m_plugin; 
		m_plugin = 0;
	}
	
	CScriptUploadEngine* newPlugin = new CScriptUploadEngine(name);
	newPlugin->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	if(newPlugin->load(m_ScriptsDirectory+name+".nut", params))
	{
		m_plugin = newPlugin; 
		return newPlugin;
	}
	else 
		delete newPlugin;
	return NULL;
}

CPluginManager::~CPluginManager()
{
	delete m_plugin;
}

void CPluginManager::UnloadPlugins()
{
	if(m_plugin)
	delete m_plugin;
	m_plugin = NULL;
}

void CPluginManager::setScriptsDirectory(const Utf8String & directory)
{
	m_ScriptsDirectory = directory;
}