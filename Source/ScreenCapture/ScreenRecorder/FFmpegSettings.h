#pragma once

#include <string>

class FFmpegSettings
{
public:
    std::string source;
    std::string codec;
    std::string preset;
    std::string outDirectory;
    bool showCursor = true;
    int quality = 20;
    int framerate = 30;

    int offsetX = 0;
    int offsetY = 0;
    int width = 0;
    int height = 0;
};