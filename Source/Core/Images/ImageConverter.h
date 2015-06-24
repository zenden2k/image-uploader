#ifndef IU_IMAGECONVERTER_H
#define IU_IMAGECONVERTER_H

#include "Thumbnail.h"
#include "Core/Utils/CoreTypes.h"
#include <string>

class AbstractImage;

struct ImageConvertingParams
{
    enum ImageResizeMode { irmFit,  irmCrop, irmStretch };
    ImageConvertingParams();

    std::string strNewWidth, strNewHeight;
    bool AddText;
    std::string Text;
    int Format;
    int Quality;
    bool SaveProportions;
#ifdef _WIN32
    LOGFONT Font;
#endif
    bool AddLogo;
    int LogoPosition;
    int LogoBlend;
    int TextPosition;
    bool SmartConverting;
    std::string LogoFileName;
    uint32_t TextColor, StrokeColor;
    EnumWrapper<ImageResizeMode> ResizeMode;
    bool PreserveExifInformation;
};

struct ThumbCreatingParams
{
    enum ThumbFormatEnum { tfSameAsImageFormat = 0, tfJPEG, tfPNG, tfGIF };
    enum ThumbResizeEnum { trByWidth = 0, trByHeight, trByBiggerSide };

    unsigned int Quality;
    std::string Text;
    std::string TemplateName;
    int Size;
    bool DrawFrame;
    bool AddImageSize;
    uint32_t BackgroundColor;
    EnumWrapper<ThumbFormatEnum> Format;
    ThumbResizeEnum ResizeMode;
};

class ImageConverterPrivate;

class ImageConverter
{
    public:
        ImageConverter();
        ~ImageConverter();
        bool Convert(const std::string& sourceFile);
        std::string getThumbFileName();
        std::string getImageFileName();
        void setDestinationFolder(const std::string& folder);
        void setGenerateThumb(bool generate);
        void setEnableProcessing(bool enable);
        void setImageConvertingParams(const ImageConvertingParams& params);
        void setThumbCreatingParams(const ThumbCreatingParams& params);
        void setThumbnail(Thumbnail* thumb);
        std::shared_ptr<AbstractImage> createThumbnail(AbstractImage* image, int64_t fileSize, int fileformat);
    protected:
        ImageConverterPrivate* d_ptr;
        Q_DECLARE_PRIVATE(ImageConverter);
private:
    DISALLOW_COPY_AND_ASSIGN(ImageConverter);

};
#endif
