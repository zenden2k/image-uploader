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

#include "GlobalMutex.h"
#include <windows.h>

namespace IuCoreUtils
{
	ZGlobalMutex::ZGlobalMutex(const std::string &name)
	{
		m_data = ::CreateMutexA(NULL, TRUE, name.c_str());
		if(!m_data && GetLastError() == ERROR_ALREADY_EXISTS)
		{
			m_data = OpenMutexA(0, 0, name.c_str());
		}
		lock();
	}

	void ZGlobalMutex::lock()
	{
		WaitForSingleObject(m_data, INFINITE); 
	}

	void ZGlobalMutex::unlock()
	{
		ReleaseMutex(reinterpret_cast<HANDLE>(m_data));
	}

	ZGlobalMutex::~ZGlobalMutex()
	{
		unlock();
		CloseHandle(reinterpret_cast<HANDLE>(m_data));
	}
}