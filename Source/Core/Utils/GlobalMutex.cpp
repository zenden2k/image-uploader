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

#include "GlobalMutex.h"
#include <windows.h>

namespace IuCoreUtils {

ZGlobalMutex::ZGlobalMutex(const std::string& name)
{
	m_data = ::CreateMutexA(NULL, TRUE, name.c_str());
	if (!m_data && GetLastError() == ERROR_ALREADY_EXISTS)
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
