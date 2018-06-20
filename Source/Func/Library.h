/*

Image Uploader -  free application for uploading images/files to the Internet

Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

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
    operator bool() const {
        return dll_ != nullptr;
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