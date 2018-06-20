#ifndef IU_CORE_IMAGES_IMAGECONVERTERPRIVATEBASE_H
#define IU_CORE_IMAGES_IMAGECONVERTERPRIVATEBASE_H

#pragma once

#include "Core/Utils/CoreTypes.h"
#include "ImageConverter.h"
#include <string>
#include <map>

class ImageConverterPrivateBase {
public:
    ImageConverterPrivateBase();
    Thumbnail* thumbnailTemplate_;
    std::map<std::string, std::string> m_Vars;
    int EvaluateExpression(const std::string& expr);
    std::string ReplaceVars(const std::string& expr);
    int64_t EvaluateSimpleExpression(const std::string& expr) const;
    uint32_t EvaluateColor(const std::string& expr);

    std::string destinationFolder_;
    bool generateThumb_;
    std::string sourceFile_;
    std::string resultFileName_;
    std::string thumbFileName_;
    ImageConvertingParams m_imageConvertingParams;
    ThumbCreatingParams m_thumbCreatingParams;
    bool processingEnabled_;
};

#endif