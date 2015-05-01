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

#include "ImageConverter.h"

#ifdef IU_WTL
    #include "ImageConverterPrivate_gdiplus.h"
#endif

using namespace Gdiplus;
#include "Func/WinUtils.h"

ImageConvertingParams::ImageConvertingParams()
{
	StrokeColor = RGB(0, 0, 0);
	SmartConverting = false;
	AddLogo  = false;
	AddText = false;
	Format = 1;
	Quality = 85;
	SaveProportions = true;
	ResizeMode = irmFit;
	LogoPosition = 0;
	LogoBlend = 0;
	Text = APPNAME;
	TextPosition = 5;
	TextColor = 0x00ffffff;
	WinUtils::StringToFont(_T("Tahoma,8,,204"), &Font);
	PreserveExifInformation = true;
}

ImageConverter::ImageConverter() : d_ptr(new ImageConverterPrivate())
{

}

ImageConverter::~ImageConverter()
{
    delete d_ptr;
}

bool ImageConverter::Convert(const std::string& sourceFile)
{
    Q_D(ImageConverter);
    return d->Convert(sourceFile);
}

void ImageConverter::setDestinationFolder(const std::string& folder)
{
    Q_D(ImageConverter);
	d->destinationFolder_ = folder;
}

void ImageConverter::setGenerateThumb(bool generate)
{
    Q_D(ImageConverter);
    d->generateThumb_ = generate;
}

void ImageConverter::setThumbnail(Thumbnail* thumb)
{
    Q_D(ImageConverter);
    d->thumbnailTemplate_ = thumb;
}

std::shared_ptr<AbstractImage> ImageConverter::createThumbnail(AbstractImage* image, int64_t fileSize, int fileformat)
{
    Q_D(ImageConverter);
    return d->createThumbnail(image, fileSize, fileformat);
}

void ImageConverter::setImageConvertingParams(const ImageConvertingParams& params)
{
    Q_D(ImageConverter);
    d->m_imageConvertingParams = params;
}

void ImageConverter::setThumbCreatingParams(const ThumbCreatingParams& params)
{
    Q_D(ImageConverter);
    d->m_thumbCreatingParams = params;
}

const std::string ImageConverter::getThumbFileName()
{
    Q_D(ImageConverter);
    return d->thumbFileName_;
}

const std::string ImageConverter::getImageFileName()
{
    Q_D(ImageConverter);
    return d->resultFileName_;
}

void ImageConverter::setEnableProcessing(bool enable)
{
    Q_D(ImageConverter);
    d->processingEnabled_ = enable;
}