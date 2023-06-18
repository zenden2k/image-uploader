#ifndef IU_CORE_UTILS_CONSOLEUTILS_H
#define IU_CORE_UTILS_CONSOLEUTILS_H

#include "Singleton.h"
#include <mutex>
#include <string>
#include <unordered_map>
#ifdef _WIN32
#include <Windows.h>
#endif

class ConsoleUtils: public Singleton<ConsoleUtils> {
public:
    enum class Color {
        Blue,
        Green,
        Red
    };
    ConsoleUtils();
    void initScreen();
    void clear();
    void setCursorPos(int x, int y);

    // Print utf-8 encoded string
    void printUnicode(FILE *f, const std::string& str);
    void printColoredText(FILE* f, const std::string& str, Color color);
    void clearLine(FILE* f) const;
    std::mutex& getOutputMutex();

protected:
    std::mutex outputMutex_;
    bool supportsEscapeCodes_ = false;
    const std::unordered_map<Color, int> codes_ = {
        {Color::Blue, 34},
        {Color::Green, 32},
        {Color::Red, 31},
    };

#ifdef _WIN32
    const std::unordered_map<Color, WORD> attributes_ = {
        {Color::Blue, FOREGROUND_BLUE},
        {Color::Green, FOREGROUND_GREEN},
        {Color::Red, FOREGROUND_RED},
    };
#endif
};

#endif // CONSOLEUTILS_H
