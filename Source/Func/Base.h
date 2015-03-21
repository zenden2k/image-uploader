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

#ifndef IU_BASE_H
#define IU_BASE_H

#include <map>
#include <vector>
#include "HistoryManager.h"

class ZBase
{
	public:
		ZBase();
		static ZBase* get();
		CHistoryManager* historyManager();
		void addToGlobalCache(const std::string fileName, const std::string url);
		std::string getFromCache(const std::string url);

	private:
		static ZBase* m_base;
		CHistoryManager m_hm;
		std::map<std::string, std::string> m_UrlCache;
};
#endif
