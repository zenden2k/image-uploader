#pragma once

#include <string>
#include <optional>

#include "Common.h"

class FFmpegOptions
{
public:
    std::string source;
    std::string audioSource;

    std::string codec;
    std::string audioCodec;

    std::string preset;
    std::string outDirectory;
    bool showCursor = true;
    int quality = 80;
    int bitrate = 3000;
    bool useQuality = true;

    int framerate = 30;
    std::string audioQuality;

    int offsetX = 0;
    int offsetY = 0;
    int width = 0;
    int height = 0;
    int outputIdx = 0;
    std::optional<int> hwAdapterIndex;
};
