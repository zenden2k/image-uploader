#pragma once

#include <string>

#include "Core/Utils/CoreTypes.h"
#include "Core/BasicConstants.h"

struct ImageConvertingParams {
    enum ImageResizeMode { irmFit,
        irmCrop,
        irmStretch };

    std::string strNewWidth, strNewHeight;
    bool AddText;
    std::string Text;
    int Format;
    int Quality;
    bool SaveProportions;
    std::string Font;
    bool AddLogo;
    int LogoPosition;
    int LogoBlend;
    int TextPosition;
    bool SmartConverting;
    std::string LogoFileName;
    uint32_t TextColor, StrokeColor;
    EnumWrapper<ImageResizeMode> ResizeMode;
    bool PreserveExifInformation;
    bool SkipAnimated;

    ImageConvertingParams()
    {
        StrokeColor = 0 /*RGB(0, 0, 0)*/;
        SmartConverting = false;
        AddLogo = false;
        AddText = false;
        Format = 1;
        Quality = 85;
        SaveProportions = true;
        ResizeMode = irmFit;
        LogoPosition = 0;
        LogoBlend = 0;
        Text = APP_NAME_A;
        TextPosition = 5;
        TextColor = 0x00ffffff;
        Font = "Tahoma,12,,204";
        PreserveExifInformation = true;
        SkipAnimated = true;
    }
};

struct ThumbCreatingParams {
    enum ThumbFormatEnum { tfSameAsImageFormat = 0,
        tfJPEG,
        tfPNG,
        tfGIF,
        tfWebP,
        tfWebPLossless };
    enum ThumbResizeEnum { trByWidth = 0,
        trByHeight,
        trByBoth };

    unsigned int Quality;
    std::string Text;
    std::string TemplateName;
    int Size;
    int Width;
    int Height;
    bool DrawFrame;
    bool AddImageSize;
    uint32_t BackgroundColor;
    EnumWrapper<ThumbFormatEnum> Format;
    ThumbResizeEnum ResizeMode;

    static const int DEFAULT_THUMB_WIDTH = 180;

    ThumbCreatingParams()
    {
        Quality = 95;
        Size = 0;
        Width = 0;
        Height = 0;
        DrawFrame = true;
        AddImageSize = true;
        BackgroundColor = 0;
        Format = tfPNG;
        ResizeMode = trByWidth;
    }
};
