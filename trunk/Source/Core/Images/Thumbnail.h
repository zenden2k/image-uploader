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

#ifndef IU_CORE_IMAGES_THUMBNAIL_H
#define IU_CORE_IMAGES_THUMBNAIL_H

#include <string>
#include <map>
#include <vector>

class Thumbnail
{
	public:
		enum PresetColorRole { kFrameColor, kStrokeColor, kGradientColor1, kTextColor };

		struct ThumbnailDrawOperation
		{
			std::string type;
			std::string rect;
			std::string brush;
			std::string source_rect;
			std::string source;
			std::string destination;
			std::string pen;
			std::string condition;
			std::string text_colors;
		};

		struct ThumbnailData
		{
			std::map<std::string, std::string> colors_;
			std::vector<ThumbnailDrawOperation> drawing_operations_;
			std::string width_addition;
			std::string height_addition;
			std::string sprite_source_file;
		};

		Thumbnail();
		virtual ~Thumbnail();
		bool LoadFromFile(const std::string& filename);
		void CreateNew();
		bool SaveToFile(const std::string& filename = "" );

		/* accessor functions */ 
		std::string getSpriteFileName() const;
		std::string getWidthAddition() const;
		std::string getHeightAddition() const;

		void setSpriteFileName(const std::string& name);

		bool existsParam(const std::string& name) const;
		unsigned int getColor(const std::string& name);
		unsigned int getParam(const std::string& name);

		void setColor(const std::string& name, unsigned int value);
		void setParam(const std::string& name, unsigned int value);
		void setParamString(const std::string& name, const std::string& value);
		std::string getParamString(const std::string& name);
		const ThumbnailData* getData() const;
	private:
		ThumbnailData data_;
		std::string file_name_;
};
#endif
