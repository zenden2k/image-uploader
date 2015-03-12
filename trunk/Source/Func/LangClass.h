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

#ifndef LANGCLASS_H
#define LANGCLASS_H
#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <atlcoll.h>

class CLang
{
	public:
		CLang();
		~CLang();
		LPTSTR GetString(LPCTSTR Name);
		bool SetDirectory(LPCTSTR Directory);
		bool LoadLanguage(LPCTSTR Lang);
		CString GetLanguageName();
		CString getLanguage() const;
		CString getLocale() const;
	private:
		struct TranslateListItem
		{
			int Hash;
			TCHAR *Name;
			TCHAR *Text;
		};
		TCHAR m_Directory[MAX_PATH];
		CString m_sLang;
		CAtlArray<TranslateListItem> StringList;
		CAtlArray<CString> LanguagesList;
		CString locale;
		CString language;
};
extern CLang Lang;

// Begin: translation macros
#define TR(str) Lang.GetString(_T(str))
#define TRC(c, str) SetDlgItemText(c, Lang.GetString(_T(str)))
// End

#endif  // LANGCLASS_H
