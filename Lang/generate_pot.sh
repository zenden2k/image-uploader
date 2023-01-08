#!/bin/bash
find ../Source/ -type d \( -path  "../Source/qimageuploader" -o -path  "../Source/QtIUHelper" -\) -prune -o -type f \( -iname \*.cpp -o -iname \*.h \)  -print | xargs -d '\n' xgettext   --from-code=UTF-8 --default-domain=imageuploader --keyword=_ --keyword=tr --keyword=TR --keyword=TRC:2 --keyword=TR_CONST --language=C++ --sort-output -o imageuploader.pot
