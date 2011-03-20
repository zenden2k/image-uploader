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

#pragma once
#include <windows.h>
#include "MyUtils.h"
#include "Core/Network/NetworkManager.h"
#include "Common.h"
#include "Core\Upload\ScriptUploadEngine.h"
#include "Core\Utils\CoreUtils.h"
CString IU_md5_file(const CString& filename);




struct ServerSettingsStruct;


class CPluginManager
{
	public:
		CPluginManager();
		~CPluginManager();
		void UnloadPlugins();
		void setScriptsDirectory(const Utf8String & directory);
		CScriptUploadEngine* getPlugin(Utf8String name, ServerSettingsStruct& params, bool UseExisting = false);
	protected:
		CScriptUploadEngine* m_plugin;
		Utf8String m_ScriptsDirectory;
};