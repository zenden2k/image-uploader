#ifndef IU_IMAGECONVERTER_H
#define IU_IMAGECONVERTER_H

#include <memory>
#include <string>

#include "ImageParams.h"
#include "Thumbnail.h"
#include "Core/Utils/CoreTypes.h"

class AbstractImage;

struct SupposedFormat {
    std::string fileName;
    std::string mimeType;
    int64_t fileSize = -1;
};

class ImageConverterPrivate;

class ImageConverter
{
    public:
        ImageConverter();
        ~ImageConverter();
        bool convert(const std::string& sourceFile);
        std::string getThumbFileName();
        std::string getImageFileName();
        void setDestinationFolder(const std::string& folder);
        void setGenerateThumb(bool generate);
        void setEnableProcessing(bool enable);
        void setImageConvertingParams(const ImageConvertingParams& params);
        void setThumbCreatingParams(const ThumbCreatingParams& params);
        void setThumbnail(Thumbnail* thumb);
        bool supposedOutputFormat(SupposedFormat& fileFormat);
        std::unique_ptr<AbstractImage> createThumbnail(AbstractImage* image, int64_t fileSize, int fileformat);
    protected:
        ImageConverterPrivate* d_ptr;
        MY_DECLARE_PRIVATE(ImageConverter);
private:
    DISALLOW_COPY_AND_ASSIGN(ImageConverter);
};
#endif
