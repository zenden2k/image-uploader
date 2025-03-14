#include "ConsoleUtils.h"

#include <cstdio>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "Core/Utils/CoreUtils.h"

ConsoleUtils::ConsoleUtils() {
#if _WIN32
    supportsEscapeCodes_ = false;
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD mode = 0;
    GetConsoleMode(hStdOut, &mode);
    if (SetConsoleMode(hStdOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0) {
        supportsEscapeCodes_ = true;
    }
#else
    supportsEscapeCodes_ = true;
#endif
}

void ConsoleUtils::initScreen() {
    //initscr();
}

void ConsoleUtils::clear() {
    std::lock_guard<std::mutex> guard(outputMutex_);
#ifdef _WIN32
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {0, 0};
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    FillConsoleOutputCharacter(hStdOut, ' ',
                               csbi.dwSize.X * csbi.dwSize.Y,
                               coord, &count);
    SetConsoleCursorPosition(hStdOut, coord);
    /* Fill the entire buffer with the current colors and attributes *
      if (!FillConsoleOutputAttribute(
        hStdOut,
        csbi.wAttributes,
        cellCount,
        homeCoords,
        &count
        )) return;*/
#else
    write(1,"\E[H\E[2J",7);
#endif
}

void ConsoleUtils::setCursorPos(int x, int y) {
    std::lock_guard<std::mutex> guard(outputMutex_);
#ifdef _WIN32
    COORD Coord;

    Coord.X = x;
    Coord.Y = y;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Coord);
#else
     printf("%c[%d;%df",0x1B,y,x);
    //printf("\033[%d;%dH", x+1, y+1);
    fflush(stdout);
    //move(x,y);
    //refresh();
#endif
}

void ConsoleUtils::printUnicode(FILE* f, const std::string& str) {
#ifdef _WIN32
    fwprintf(f, L"%s", IuCoreUtils::Utf8ToWstring(str).c_str());
#else
    fprintf(f, "%s", str.c_str());
#endif
    fflush(f);
}

void ConsoleUtils::printColoredText(FILE* f, const std::string& str, Color color) {
#ifdef _WIN32
    if (!supportsEscapeCodes_) {
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(hStdOut, &info);
        auto it = attributes_.find(color);
        if (it != attributes_.end()) {
            SetConsoleTextAttribute(hStdOut, it->second);
        }
        printUnicode(f, str);
        if (it != attributes_.end()) {
            SetConsoleTextAttribute(hStdOut, info.wAttributes);
        }
        return;
    }
#endif
    auto it = codes_.find(color);
    if (supportsEscapeCodes_ && it != codes_.end()) {
        fprintf(f, "\033[%dm", static_cast<int>(it->second));  
    }

    printUnicode(f, str);

    if (supportsEscapeCodes_ && it != codes_.end()) {
        fprintf(f, "\033[0m");
    }
}

void ConsoleUtils::clearLine(FILE* f) const {
    if (supportsEscapeCodes_) {
        fprintf(f, "\33[2K");
    }
}

std::mutex& ConsoleUtils::getOutputMutex() {
    return outputMutex_;
}

