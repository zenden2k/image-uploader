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

#include "langclass.h"
#include "atlheaders.h"
#include "myutils.h"
#include <Func/WinUtils.h>

CLang Lang;

// TODO: rewrite this shit
// check it's compatibility with 64 bit platforms
BYTE hex_digit(TCHAR f)
{
	BYTE p;
	if (f >= _T('0') && f <= _T('9'))
	{
		p = f - _T('0');
	}
	else
	{
		p = f - _T('a') + 10;
	}
	return p;
}

int hexstr2int(LPTSTR hex)
{
	int len = lstrlen(hex);
	int res = 0, step = 1;
	int c = 0;

	BYTE b[4];
	if (len > 8)
		return 0;
	for (int i = 0; i < len; i += 2)
	{
		ATLASSERT(i / 2 <= 3);
		b[i / 2] = hex_digit(hex[i]) * 16 + hex_digit(hex[i + 1]);
		step *= 16;
	}
	return *(DWORD*)&b;
}

int myhash(PBYTE key, int len)
{
	int hash = 222;
	for (int i = 0; i < len; ++i)
	{
		hash = (hash ^ key[i]) + ((hash << 26) + (hash >> 6));
	}

	return hash;
}

CLang::CLang()
{
	*m_Directory = 0;
}

bool CLang::SetDirectory(LPCTSTR Directory)
{
	lstrcpyn(m_Directory, Directory, sizeof(m_Directory) / sizeof(TCHAR));
	return true;
}

bool CLang::LoadLanguage(LPCTSTR Lang)
{
	StringList.RemoveAll();
	if (!Lang)
		return false;

	CString Filename = CString(m_Directory) + Lang + _T(".lng");

	FILE* f = _tfopen(Filename, _T("rb"));
	if (!f)
		return false;

	fseek(f, 2, 0);
	TCHAR Buffer[1024];
	TCHAR Name[128];
	TCHAR Text[512];

	while (!feof(f))
	{
		*Buffer = *Name = *Text = 0;
		fgetline(Buffer, sizeof(Buffer) / sizeof(TCHAR), f);

		if (*Buffer == _T('#'))
			continue;
		WinUtils::ExtractStrFromList(Buffer, 0, Name, sizeof(Name) / sizeof(TCHAR), _T(""), _T('='));
		WinUtils::ExtractStrFromList(Buffer, 1, Text, sizeof(Text) / sizeof(TCHAR), _T(""), _T('='));

		if (Name[lstrlen(Name) - 1] == _T(' '))
			Name[lstrlen(Name) - 1] = 0;

		CString RepText = Text;
		RepText.Replace(_T("\\n"), _T("\n"));

		int NameLen = lstrlen(Name);
		int TextLen = lstrlen(RepText);

		if (!NameLen || !TextLen)
			continue;

		TCHAR* pName = new TCHAR[NameLen + 1];
		if (!pName)
			return false;
		TCHAR* pText = new TCHAR[TextLen + 1];
		if (!pText)
			return false;

		lstrcpy(pName, Name);
		lstrcpy(pText, (LPCTSTR)RepText + ((*Text == _T(' ')) ? 1 : 0));

		TranslateListItem tli = {NULL, NULL};
		tli.Name = pName;
		tli.Text = pText;

		tli.Hash = hexstr2int(pName);
		StringList.Add(tli);
	}

	fclose(f);
	m_sLang = Lang;
	return true;
}

LPTSTR CLang::GetString(LPCTSTR Name)
{
	int n = StringList.GetCount();

	for (int i = 0; i < n; i++)
	{
		if (StringList[i].Hash == myhash((PBYTE) Name, lstrlen( Name) * sizeof(TCHAR)))
			return StringList[i].Text;
	}

	// return _T("$NO_SUCH_STRING");
	return (LPTSTR)Name;
}

CString CLang::GetLanguageName()
{
	return m_sLang;
}

CLang::~CLang()
{
}
