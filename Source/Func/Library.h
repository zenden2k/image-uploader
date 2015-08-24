#ifndef IU_FUNC_LIBRARY_H
#define IU_FUNC_LIBRARY_H

#pragma once 

#include <windows.h>
#include <mutex>

class Library {
public:
    Library(const wchar_t* libraryName) : libraryName_(libraryName), dll_(NULL) {
        libraryName_ = libraryName;
    }
    ~Library() {
        if (dll_) {
            std::lock_guard<std::mutex> guard(mutex_);
            if (dll_) {
                FreeLibrary(dll_);
            }
        }
    }
    template<class T> T GetProcAddress(const char* func) {
        if (!EnsureLoaded()) {
            return nullptr;
        }
        return reinterpret_cast<T>(::GetProcAddress(dll_, func));
    }
private:
    std::wstring libraryName_;
    std::mutex mutex_;
    HMODULE dll_; 
    bool isLoaded_;

    bool EnsureLoaded() {
        if (!dll_) {
            std::lock_guard<std::mutex> guard(mutex_);
            if (!dll_) {
                dll_ = ::LoadLibraryW(libraryName_.c_str());
                if (!dll_) {
                    return false;
                }
            }
        }
        return true;
    }
};

#endif