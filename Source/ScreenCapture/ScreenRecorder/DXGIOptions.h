#pragma once

#include <string>
#include <vector>

#include "Common.h"

class DXGIOptions
{
public:
    std::string source;
    std::vector<std::string> audioSources;

    std::string codec;
    std::string audioCodec;

    std::string preset;
    std::string outDirectory;
    bool showCursor = true;
    int quality = 80;
    int bitrate = 3000;
    bool useQuality = true;

    int framerate = 30;
    int audioBitrate;

    int offsetX = 0;
    int offsetY = 0;
    int width = 0;
    int height = 0;
};
