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
#include "CmdLine.h"
#include "../Func/MyUtils.h"
CCmdLine::CCmdLine()
{
	Parse(GetCommandLine());
}

CCmdLine::CCmdLine(LPCTSTR szCmdLine)
{
	Parse(szCmdLine);
}

void CCmdLine::Parse(LPCTSTR szCmdLine)
{
	m_Params.RemoveAll();
	LPCTSTR Cur = szCmdLine;
	TCHAR Buffer[MAX_PATH];

	while(Cur && *Cur)
	{
		*Buffer=0;
		if(*Cur=='"') 
			Cur = CopyToStartOfW(++Cur, _T("\""), Buffer, sizeof(Buffer)/sizeof(TCHAR));
		else 
			Cur = CopyToStartOfW(Cur, _T(" "), Buffer, sizeof(Buffer)/sizeof(TCHAR));

		AddParam(Buffer);
		while(*Cur && *Cur==' ') Cur++;
		if(m_Params.GetCount()==1)
			m_OnlyParams=Cur;
	}
	return;
}

int CCmdLine::AddParam(LPCTSTR szParam)
{
	m_Params.Add(szParam);
	return m_Params.GetCount()-1;
}

CString CCmdLine::operator[](int nIndex)
{
	if(nIndex < 0 || nIndex > int(m_Params.GetCount())-1)
		return _T("");
	return m_Params[nIndex];
}

CString CCmdLine::OnlyParams()
{
	return m_OnlyParams;
}

CString CCmdLine::ModuleName()
{
	return m_Params.GetCount()? m_Params[0] : _T("");
}

bool CCmdLine::GetNextFile(CString &FileName, int &nIndex)
{
	for(size_t i=nIndex+1; i<m_Params.GetCount(); i++)
	{
		if(m_Params[i].GetLength() && m_Params[i][0]!=_T('/'))
		{
			FileName = m_Params[i];
			nIndex = i;
			return true;
		}
	}
	return false;
}

CCmdLine& CCmdLine::operator=(const CCmdLine &p)
{
	m_Params.RemoveAll();
	m_Params.Copy(p.m_Params);
	return *this;
}

bool CCmdLine::IsOption(LPCTSTR Option, bool bUsePrefix)
{
	CString Temp;
	if(bUsePrefix) Temp = CString("/"); 
	Temp += Option;
	for(size_t i=1; i<m_Params.GetCount(); i++)
	{
		if(lstrcmpi(m_Params[i], Temp)==0)
			return true;
	}
	return false;
}

CString IntToStr(int n)
{
	CString Result;
	Result.Format(_T("%d"), n);
	return Result;
}

size_t CCmdLine::GetCount()
{
	return m_Params.GetCount();
}

CCmdLine CmdLine;