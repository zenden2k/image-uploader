/*

Image Uploader -  free application for uploading images/files to the Internet

Copyright 2007-2024 Sergey Svistunov (zenden2k@gmail.com)

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

#pragma once

#include "Core/Upload/UploadFilter.h"

class ImageSearchFilter : public UploadFilter
{
public:
    bool PreUpload(UploadTask* task) override;
    bool PostUpload(UploadTask* task) override;
};

