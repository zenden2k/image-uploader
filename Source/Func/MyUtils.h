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

#ifndef _MYUTILS_H_
#define _MYUTILS_H_

#include <atlheaders.h>
#include <tchar.h>

//#define VIDEO_DIALOG_FORMATS _T("Video files (avi, mpg, vob, wmv, flv, etc)\0*.avi;*.mpeg;*.mpg;*.mp2;*.divx;*.vob;*.flv;*.wmv;*.asf;*.mkv;*.mp4;*.ts;*.mov;*.mpeg2ts;*.3gp;\0All files\0*.*\0\0")
#define VIDEO_FORMATS _T("avi\0mpg\0mpeg\0vob\0divx\0flv\0wmv\0asf\0mkv\0mov\0ts\0mp2\0mp4\0") _T("3gp\0rm\0mpeg2ts\0\0")
#define IMAGE_DIALOG_FORMATS _T("Image files (JPEG, GIF, PNG, etc)\0*.jpg;*.gif;*.png;*.bmp;*.tiff\0All files\0*.*\0\0")

#define xor(a,b) ((a || b) && !(a && b))    

#define CheckBounds(n,a,b,d) {if((n<a) || (n>b)) n=d;}

#define LOADICO(ico) LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(ico))

int GetFontSizeInTwips(int nFontSize);


bool IsVideoFile(LPCTSTR szFileName);
int GetSavingFormat(LPCTSTR szFileName);

LPTSTR fgetline(LPTSTR buf,int num,FILE *f);
CString TrimString(const CString& source, int nMaxLen);

LPCTSTR  CopyToStartOfW(LPCTSTR szString,LPCTSTR szPattern,LPTSTR szBuffer,int nBufferSize);

HFONT MakeFontUnderLine(HFONT font);
HFONT MakeFontBold(HFONT font);
LPTSTR MoveToEndOfW(LPTSTR szString,LPTSTR szPattern);

#ifndef IU_SHELLEXT


#define PROP_OBJECT_PTR            MAKEINTATOM(ga.atom)
#define PROP_ORIGINAL_PROC        MAKEINTATOM(ga.atom)

/*
 * typedefs
 */
class CGlobalAtom
{
public:
    CGlobalAtom(void)
    { atom = GlobalAddAtom(TEXT("_Hyperlink_Object_Pointer_")
             TEXT("\\{AFEED740-CC6D-47c5-831D-9848FD916EEF}")); }
    ~CGlobalAtom(void)
    { DeleteAtom(atom); }

    ATOM atom;
};

/*
 * Local variables
 */
static CGlobalAtom ga;
#endif

bool CheckFileName(const CString& fileName);

#endif


