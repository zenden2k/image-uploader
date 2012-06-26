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

#ifndef IU_CORE_UTILS_CRYPTOUTILS_H
#define IU_CORE_UTILS_CRYPTOUTILS_H

#include <cstdio>
#include <string>
#include <vector>
#include "CoreTypes.h"

namespace IuCoreUtils {

class CryptoUtils {
	public:
		static const std::string CalcMD5Hash(const void* data, size_t size);
		static const std::string CalcMD5HashFromString(const std::string &data);
		static const std::string CalcMD5HashFromFile(const std::string& filename);

		static const std::string CalcSHA1Hash(const void* data, size_t size);
		static const std::string CalcSHA1HashFromString(const std::string& data);
		static const std::string CalcSHA1HashFromFile(const std::string& filename);
};

};

#endif // IU_CORE_UTILS_CRYPTOUTILS_H
