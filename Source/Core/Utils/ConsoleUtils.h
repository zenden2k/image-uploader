#ifndef IU_CORE_UTILS_CONSOLEUTILS_H
#define IU_CORE_UTILS_CONSOLEUTILS_H

#include "Singleton.h"
#include <mutex>


class ConsoleUtils: public Singleton<ConsoleUtils> {
public:
    void InitScreen();
    void Clear();
    void SetCursorPos(int x, int y);
    std::mutex& getOutputMutex();
protected:
    std::mutex outputMutex_;
};

#endif // CONSOLEUTILS_H
