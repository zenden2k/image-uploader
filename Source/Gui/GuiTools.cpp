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

#include "GuiTools.h"

namespace ZGuiTools
{
	int AddComboBoxItem(HWND hDlg, int itemId, LPCTSTR item)
	{
		return ::SendDlgItemMessage(hDlg, itemId, CB_ADDSTRING, 0, (LPARAM)item);
	}

	bool AddComboBoxItems(HWND hDlg, int itemId, int itemCount, LPCTSTR item, ...)
	{
		bool result = true;
		for(int i=0; i<itemCount; i++)
		{
			if(AddComboBoxItem(hDlg, itemId, *(&item + i)) < 0)
				result = false;
		}
		return result;
	}
};