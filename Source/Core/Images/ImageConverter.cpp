/*

    Uptooda - free application for uploading images/files to the Internet

    Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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

#include "Core/CommonDefs.h"
#include "Core/Utils/CoreUtils.h"

#ifdef _WIN32
    #include "ImageConverterPrivate_gdiplus.h"
#endif

#include "Func/WinUtils.h"

ImageConverter::ImageConverter() : d_ptr(new ImageConverterPrivate())
{

}

ImageConverter::~ImageConverter()
{
    delete d_ptr;
}

bool ImageConverter::convert(const std::string& sourceFile)
{
    MY_D(ImageConverter);
    return d->convert(sourceFile);
}

void ImageConverter::setDestinationFolder(const std::string& folder)
{
    MY_D(ImageConverter);
    d->destinationFolder_ = folder;
}

void ImageConverter::setGenerateThumb(bool generate)
{
    MY_D(ImageConverter);
    d->generateThumb_ = generate;
}

void ImageConverter::setThumbnail(Thumbnail* thumb)
{
    MY_D(ImageConverter);
    d->thumbnailTemplate_ = thumb;
}

bool ImageConverter::supposedOutputFormat(SupposedFormat& fileFormat) {
    MY_D(ImageConverter);
    return d->supposedOutputFormat(fileFormat);
}

std::unique_ptr<AbstractImage> ImageConverter::createThumbnail(AbstractImage* image, int64_t fileSize, int fileformat)
{
    MY_D(ImageConverter);
    return d->createThumbnail(image, fileSize, fileformat);
}

void ImageConverter::setImageConvertingParams(const ImageConvertingParams& params)
{
    MY_D(ImageConverter);
    d->m_imageConvertingParams = params;
}

void ImageConverter::setThumbCreatingParams(const ThumbCreatingParams& params)
{
    MY_D(ImageConverter);
    d->m_thumbCreatingParams = params;
}

std::string ImageConverter::getThumbFileName()
{
    MY_D(ImageConverter);
    return d->thumbFileName_;
}

std::string ImageConverter::getImageFileName()
{
    MY_D(ImageConverter);
    return d->resultFileName_;
}

void ImageConverter::setEnableProcessing(bool enable)
{
    MY_D(ImageConverter);
    d->processingEnabled_ = enable;
}
