/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#ifndef MYDROPSOURCE_H
#define MYDROPSOURCE_H


#pragma once
#include "atlheaders.h"
class CMyDropSource : public IDropSource
{
private:
    long m_lRefCount;

public:
    CMyDropSource();
    ~CMyDropSource();

    // IUnknown members.
    HRESULT __stdcall QueryInterface(REFIID iid,void ** ppvObject);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();

    // IDropSource members.
    HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState);
    HRESULT __stdcall GiveFeedback(DWORD dwEffect);
};

#endif // MYDROPSOURCE_H