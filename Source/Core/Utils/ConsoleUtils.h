#ifndef IU_CORE_UTILS_CONSOLEUTILS_H
#define IU_CORE_UTILS_CONSOLEUTILS_H

#include "Singleton.h"
#include <mutex>
#include <string>

class ConsoleUtils: public Singleton<ConsoleUtils> {
public:
    void InitScreen();
    void Clear();
    void SetCursorPos(int x, int y);

    // Print utf-8 encoded string
    void PrintUnicode(FILE *f, const std::string& str);
    std::mutex& getOutputMutex();
protected:
    std::mutex outputMutex_;
};

#endif // CONSOLEUTILS_H
