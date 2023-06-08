#!/bin/bash
doxygen ScriptAPI.cfg
cd ../../Docs/api/html/
find . -type f -name "*.html" -exec sed -i 's/const std::string &amp\;/string /g;s/const std::string/string/g;s/std::string/string/g;s/Sqrat::Array/array/g;s/Sqrat::Table/table/g;s/UploadTaskWrapperBase/UploadTask/g;s/FileUploadTaskWrapper/FileUploadTask/g;s/UrlShorteningTaskWrapper/UrlShorteningTask/g;s/UploadTaskWrapper/UploadTask/g;s/int64_t/int/g;s/RegularExpression/CRegExp/g;s/Sqrat::Function/function/g;s/Sqrat::Object/object/g;s/=&quot;&quot;//g;s/const object &amp\;/object /g;s/ override//g;s/squirrelEach/each/g;' {} +
find . -type f -name "class*.html" -exec sed -i 's/const //g;s/ &amp;/ /g;' {} +
cd -