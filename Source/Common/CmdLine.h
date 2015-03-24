/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
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
#ifndef IU_FUNC_CMDLINE_H
#define IU_FUNC_CMDLINE_H

#pragma once
#include "atlheaders.h"

class CCmdLine
{
	public:
		CCmdLine();
		CCmdLine &operator=(const CCmdLine& p);
		explicit CCmdLine(LPCTSTR szCmdLine);
		void Parse(LPCTSTR szCmdLine);
		int AddParam(LPCTSTR szParam);

		//Returns command line without module name
		CString OnlyParams();
		CString operator[](int nIndex);
		CString ModuleName();
		bool GetNextFile(CString& FileName, int& nIndex);
		bool IsOption(LPCTSTR Option, bool bUsePrefix = true);
		size_t GetCount();
	private:
		CStringList m_Params;
		CString m_OnlyParams;
};

extern CCmdLine CmdLine;
#endif // IU_FUNC_CMDLINE_H
