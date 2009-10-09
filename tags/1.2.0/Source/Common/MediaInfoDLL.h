/* MediaInfoDLL - All info about media files, for DLL
// Copyright (C) 2002-2009 Jerome Martinez, Zen@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Public DLL interface implementation
// Wrapper for MediaInfo Library
// See MediaInfo.h for help
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef MediaInfoDLLH
#define MediaInfoDLLH

extern TCHAR MediaInfoDllPath[MAX_PATH];
/*-------------------------------------------------------------------------*/
#if defined (_WIN32) || defined (WIN32)
    #ifdef _DEBUG
        #define MEDIAINFODLL_NAME  "Modules\\MediaInfo.dll"
    #else    
        #define MEDIAINFODLL_NAME  "Modules\\MediaInfo.dll"
    #endif //_DEBUG
#elif defined(__APPLE__) && defined(__MACH__)
    #define MEDIAINFODLL_NAME  "libmediainfo.0.dylib"
    #define __stdcall
#else
    #define MEDIAINFODLL_NAME  "libmediainfo.so.0"
    #define __stdcall
#endif //!defined(_WIN32) || defined (WIN32)
#include <new> //For size_t in MacOS

/*-------------------------------------------------------------------------*/
/*Char types                                                               */
#undef  _T
#define _T(__x)     __T(__x)
#if defined(UNICODE) || defined (_UNICODE)
    typedef wchar_t MediaInfo_Char;
    #undef  __T
    #define __T(__x) L ## __x
    #define MEDIAINFO_Ansi ""
#else
    typedef char MediaInfo_Char;
    #undef  __T
    #define __T(__x) __x
    #define MEDIAINFO_Ansi "A"
#endif
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*NULL                                                                      */
#ifndef NULL
    #define NULL 0
#endif
/*-------------------------------------------------------------------------*/

/** @brief Kinds of Stream */
typedef enum MediaInfo_stream_t
{
    MediaInfo_Stream_General,
    MediaInfo_Stream_Video,
    MediaInfo_Stream_Audio,
    MediaInfo_Stream_Text,
    MediaInfo_Stream_Chapters,
    MediaInfo_Stream_Image,
    MediaInfo_Stream_Menu,
    MediaInfo_Stream_Max
} MediaInfo_stream_C;

/** @brief Kinds of Info */
typedef enum MediaInfo_info_t
{
    MediaInfo_Info_Name,
    MediaInfo_Info_Text,
    MediaInfo_Info_Measure,
    MediaInfo_Info_Options,
    MediaInfo_Info_Name_Text,
    MediaInfo_Info_Measure_Text,
    MediaInfo_Info_Info,
    MediaInfo_Info_HowTo,
    MediaInfo_Info_Max
} MediaInfo_info_C;

/** @brief Option if InfoKind = Info_Options */
typedef enum MediaInfo_infooptions_t
{
    MediaInfo_InfoOption_ShowInInform,
    MediaInfo_InfoOption_Reserved,
    MediaInfo_InfoOption_ShowInSupported,
    MediaInfo_InfoOption_TypeOfValue,
    MediaInfo_InfoOption_Max
} MediaInfo_infooptions_C;

/** @brief File opening options */
typedef enum MediaInfo_fileoptions_t
{
    MediaInfo_FileOption_Nothing        =0x00,
    MediaInfo_FileOption_Recursive      =0x01,
    MediaInfo_FileOption_CloseAll       =0x02,
    MediaInfo_FileOption_Max            =0x04
} MediaInfo_fileoptions_C;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifdef MEDIAINFO_GLIBC
    #include <gmodule.h>
    static GModule* Module=NULL;
#elif defined (_WIN32) || defined (WIN32)
    #include <windows.h>
    static HMODULE  Module=NULL;
#else
    #include <dlfcn.h>
    static void*    Module=NULL;
#endif
static size_t Module_Count=0;

#ifdef MEDIAINFO_GLIBC
#define MEDIAINFO_ASSIGN(_Name,_Name2) \
    if (!g_module_symbol (Module, "MediaInfo"MEDIAINFO_Ansi"_"_Name2, (gpointer*)&MediaInfo_##_Name)) \
        Errors++; \
    if (!g_module_symbol (Module, "MediaInfoList"MEDIAINFO_Ansi"_"_Name2, (gpointer*)&MediaInfoList_##_Name)) \
        Errors++;
#elif defined (_WIN32) || defined (WIN32)
#define MEDIAINFO_ASSIGN(_Name,_Name2) \
    MediaInfo_##_Name=(MEDIAINFO_##_Name)GetProcAddress(Module, "MediaInfo"MEDIAINFO_Ansi"_"_Name2); \
    if (MediaInfo_##_Name==NULL) Errors++; \
    MediaInfoList_##_Name=(MEDIAINFOLIST_##_Name)GetProcAddress(Module, "MediaInfoList"MEDIAINFO_Ansi"_"_Name2); \
    if (MediaInfoList_##_Name==NULL) Errors++;
#else
#define MEDIAINFO_ASSIGN(_Name,_Name2) \
    MediaInfo_##_Name=(MEDIAINFO_##_Name)dlsym(Module, "MediaInfo"MEDIAINFO_Ansi"_"_Name2); \
    if (MediaInfo_##_Name==NULL) Errors++; \
    MediaInfoList_##_Name=(MEDIAINFOLIST_##_Name)dlsym(Module, "MediaInfoList"MEDIAINFO_Ansi"_"_Name2); \
    if (MediaInfoList_##_Name==NULL) Errors++;
#endif

typedef void* (__stdcall *MEDIAINFO_New)(); static MEDIAINFO_New MediaInfo_New;
typedef void* (__stdcall *MEDIAINFOLIST_New)(); static MEDIAINFOLIST_New MediaInfoList_New;
typedef void (__stdcall *MEDIAINFO_Delete)(void*); static MEDIAINFO_Delete MediaInfo_Delete;
typedef void (__stdcall *MEDIAINFOLIST_Delete)(void*); static MEDIAINFOLIST_Delete MediaInfoList_Delete;
typedef size_t (__stdcall *MEDIAINFO_Open)(void*, const MediaInfo_Char*); static MEDIAINFO_Open MediaInfo_Open;
typedef size_t (__stdcall *MEDIAINFOLIST_Open)(void*, const MediaInfo_Char*, const MediaInfo_fileoptions_C); static MEDIAINFOLIST_Open MediaInfoList_Open;
typedef void (__stdcall *MEDIAINFO_Close)(void*); static MEDIAINFO_Close MediaInfo_Close;
typedef void (__stdcall *MEDIAINFOLIST_Close)(void*, size_t); static MEDIAINFOLIST_Close MediaInfoList_Close;
typedef const MediaInfo_Char* (__stdcall *MEDIAINFO_Inform)(void*, size_t Reserved); static MEDIAINFO_Inform MediaInfo_Inform;
typedef const MediaInfo_Char* (__stdcall *MEDIAINFOLIST_Inform)(void*, size_t, size_t Reserved); static MEDIAINFOLIST_Inform MediaInfoList_Inform;
typedef const MediaInfo_Char* (__stdcall *MEDIAINFO_GetI)(void*, MediaInfo_stream_C StreamKind, size_t StreamNumber, size_t Parameter, MediaInfo_info_C KindOfInfo); static MEDIAINFO_GetI MediaInfo_GetI;
typedef const MediaInfo_Char* (__stdcall *MEDIAINFOLIST_GetI)(void*, size_t, MediaInfo_stream_C StreamKind, size_t StreamNumber, size_t Parameter, MediaInfo_info_C KindOfInfo); static MEDIAINFOLIST_GetI MediaInfoList_GetI;
typedef const MediaInfo_Char* (__stdcall *MEDIAINFO_Get)(void*, MediaInfo_stream_C StreamKind, size_t StreamNumber, const MediaInfo_Char* Parameter, MediaInfo_info_C KindOfInfo, MediaInfo_info_C KindOfSearch); static MEDIAINFO_Get MediaInfo_Get;
typedef const MediaInfo_Char* (__stdcall *MEDIAINFOLIST_Get)(void*, size_t, MediaInfo_stream_C StreamKind, size_t StreamNumber, const MediaInfo_Char* Parameter, MediaInfo_info_C KindOfInfo, MediaInfo_info_C KindOfSearch); static MEDIAINFOLIST_Get MediaInfoList_Get;
typedef const MediaInfo_Char* (__stdcall *MEDIAINFO_Option)(void*, const MediaInfo_Char* Parameter, const MediaInfo_Char* Value); static MEDIAINFO_Option MediaInfo_Option;
typedef const MediaInfo_Char* (__stdcall *MEDIAINFOLIST_Option)(void*, const MediaInfo_Char* Parameter, const MediaInfo_Char* Value); static MEDIAINFOLIST_Option MediaInfoList_Option;
typedef size_t (__stdcall *MEDIAINFO_State_Get)(void*); static MEDIAINFO_State_Get MediaInfo_State_Get;
typedef size_t (__stdcall *MEDIAINFOLIST_State_Get)(void*); static MEDIAINFOLIST_State_Get MediaInfoList_State_Get;
typedef size_t (__stdcall *MEDIAINFO_Count_Get)(void*, MediaInfo_stream_C StreamKind, size_t StreamNumber); static MEDIAINFO_Count_Get MediaInfo_Count_Get;
typedef size_t (__stdcall *MEDIAINFOLIST_Count_Get)(void*, size_t, MediaInfo_stream_C StreamKind, size_t StreamNumber); static MEDIAINFOLIST_Count_Get MediaInfoList_Count_Get;
typedef size_t (__stdcall *MEDIAINFO_Count_Get_Files)(void*); static MEDIAINFO_Count_Get_Files MediaInfo_Count_Get_Files;
typedef size_t (__stdcall *MEDIAINFOLIST_Count_Get_Files)(void*); static MEDIAINFOLIST_Count_Get_Files MediaInfoList_Count_Get_Files;

static size_t MediaInfoDLL_Load()
{
    if (Module_Count>0)
    {
        Module_Count++;
        return 1;
    }

    /* Load library */
    #ifdef MEDIAINFO_GLIBC
        Module=g_module_open(MEDIAINFODLL_NAME, G_MODULE_BIND_LAZY);
    #elif defined (_WIN32) || defined (WIN32)
        Module=LoadLibrary(MediaInfoDllPath/*_T(MEDIAINFODLL_NAME)*/);
    #else
        Module=dlopen(MEDIAINFODLL_NAME, RTLD_LAZY);
        if (!Module)
            Module=dlopen("./"MEDIAINFODLL_NAME, RTLD_LAZY);
        if (!Module)
            Module=dlopen("/usr/local/lib/"MEDIAINFODLL_NAME, RTLD_LAZY);
        if (!Module)
            Module=dlopen("/usr/local/lib64/"MEDIAINFODLL_NAME, RTLD_LAZY);
        if (!Module)
            Module=dlopen("/usr/lib/"MEDIAINFODLL_NAME, RTLD_LAZY);
        if (!Module)
            Module=dlopen("/usr/lib64/"MEDIAINFODLL_NAME, RTLD_LAZY);
    #endif
    if (!Module)
        return (size_t)-1;

    /* Load methods */
    size_t Errors=0;
    MEDIAINFO_ASSIGN(New,"New")
    MEDIAINFO_ASSIGN(Delete,"Delete")
    MEDIAINFO_ASSIGN(Open,"Open")
    MEDIAINFO_ASSIGN(Close,"Close")
    MEDIAINFO_ASSIGN(Inform,"Inform")
    MEDIAINFO_ASSIGN(GetI,"GetI")
    MEDIAINFO_ASSIGN(Get,"Get")
    MEDIAINFO_ASSIGN(Option,"Option")
    MEDIAINFO_ASSIGN(State_Get,"State_Get")
    MEDIAINFO_ASSIGN(Count_Get,"Count_Get")
    MEDIAINFO_ASSIGN(Count_Get_Files,"Count_Get_Files")
    if (Errors>1) //Normal for Count_Get_Files, MediaInfo has no one.
       return (size_t)-1;

    Module_Count++;
    return (size_t)1;
}

static size_t MediaInfoDLL_IsLoaded()
{
    if (Module)
        return 1;
    else
        return 0;
}

static void MediaInfoDLL_UnLoad()
{
    Module_Count--;
    if (Module_Count>0)
        return;

    #ifdef MEDIAINFO_GLIBC
        g_module_close(Module);
    #elif defined (_WIN32) || defined (WIN32)
        FreeLibrary(Module);
    #else
        dlclose(Module);
    #endif
    Module=NULL;
}

#ifdef __cplusplus
}
#endif /*__cplusplus*/

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#ifdef __cplusplus
//DLL C++ wrapper for C functions

//---------------------------------------------------------------------------
/*#include <string>
#include <sstream>*/
//---------------------------------------------------------------------------

namespace MediaInfoDLL
{

//---------------------------------------------------------------------------
//MediaInfo_Char types
#undef  _T
#define _T(__x)     __T(__x)
#if defined(UNICODE) || defined (_UNICODE)
    typedef wchar_t Char;
    #undef  __T
    #define __T(__x) L ## __x
#else
    typedef char Char;
    #undef  __T
    #define __T(__x) __x
#endif
/*typedef std::basic_string<Char>        CString;
typedef std::basic_stringstream<Char>  StringStream;
typedef std::basic_istringstream<Char> tiStringStream;
typedef std::basic_ostringstream<Char> toStringStream;*/
const size_t Error=(size_t)(-1);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/// @brief Kinds of Stream
enum stream_t
{
    Stream_General,                 ///< StreamKind = General
    Stream_Video,                   ///< StreamKind = Video
    Stream_Audio,                   ///< StreamKind = Audio
    Stream_Text,                    ///< StreamKind = Text
    Stream_Chapters,                ///< StreamKind = Chapters
    Stream_Image,                   ///< StreamKind = Image
    Stream_Menu,                    ///< StreamKind = Menu
    Stream_Max,
};

/// @brief Kind of information
enum info_t
{
    Info_Name,                      ///< InfoKind = Unique name of parameter
    Info_Text,                      ///< InfoKind = Value of parameter
    Info_Measure,                   ///< InfoKind = Unique name of measure unit of parameter
    Info_Options,                   ///< InfoKind = See infooptions_t
    Info_Name_Text,                 ///< InfoKind = Translated name of parameter
    Info_Measure_Text,              ///< InfoKind = Translated name of measure unit
    Info_Info,                      ///< InfoKind = More information about the parameter
    Info_HowTo,                     ///< InfoKind = Information : how data is found
    Info_Max
};

/// Get(...)[infooptions_t] return a string like "YNYN..." \n
/// Use this enum to know at what correspond the Y (Yes) or N (No)
/// If Get(...)[0]==Y, then :
/// @brief Option if InfoKind = Info_Options
enum infooptions_t
{
    InfoOption_ShowInInform,        ///< Show this parameter in Inform()
    InfoOption_Reserved,            ///<
    InfoOption_ShowInSupported,     ///< Internal use only (info : Must be showed in Info_Capacities() )
    InfoOption_TypeOfValue,         ///< Value return by a standard Get() can be : T (Text), I (Integer, warning up to 64 bits), F (Float), D (Date), B (Binary datas coded Base64) (Numbers are in Base 10)
    InfoOption_Max
};

/// @brief File opening options
enum fileoptions_t
{
    FileOption_Nothing      =0x00,
    FileOption_Recursive    =0x01,  ///< Browse folders recursively
    FileOption_CloseAll     =0x02,  ///< Close all files before open
    FileOption_Max          =0x04
};

const CString Unable_Load_DLL=_T("Unable to load ")_T(MEDIAINFODLL_NAME);
#define MEDIAINFO_TEST_VOID \
    if (!IsReady()) return
#define MEDIAINFO_TEST_INT \
    if (!IsReady()) return 0
#define MEDIAINFO_TEST_STRING \
    if (!IsReady()) return Unable_Load_DLL
#define MEDIAINFO_TEST_STRING_STATIC \
    if (!Module) return Unable_Load_DLL

//---------------------------------------------------------------------------
class MediaInfo
{
public :
    MediaInfo ()                {if (!Module) MediaInfoDLL_Load(); if (!Module) {Handle=NULL; return;}; Handle=MediaInfo_New();};
    ~MediaInfo ()               {MEDIAINFO_TEST_VOID; MediaInfo_Delete(Handle);};

    //File
    size_t Open (const CString &File) {MEDIAINFO_TEST_INT; return MediaInfo_Open(Handle, File);};
    //size_t Open (const unsigned char* Begin, size_t Begin_Size, const unsigned char* End=NULL, size_t End_Size=NULL) {MEDIAINFO_TEST_INT; return MediaInfo_Open_Buffer(Handle, Begin, Begin_Size, End, End_Size);};
    //size_t Save () {MEDIAINFO_TEST_INT; return MediaInfo_Save(Handle);};
    void Close () {MEDIAINFO_TEST_VOID; return MediaInfo_Close(Handle);};

    //General information
    CString Inform ()  {MEDIAINFO_TEST_STRING; return MediaInfo_Inform(Handle, 0);};
    CString Get (stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t InfoKind=Info_Text)  {MEDIAINFO_TEST_STRING; return MediaInfo_GetI (Handle, (MediaInfo_stream_C)StreamKind, StreamNumber, Parameter, (MediaInfo_info_C)InfoKind);};
    CString Get (stream_t StreamKind, size_t StreamNumber, const CString &Parameter, info_t InfoKind=Info_Text, info_t SearchKind=Info_Name)  {MEDIAINFO_TEST_STRING; return MediaInfo_Get (Handle, (MediaInfo_stream_C)StreamKind, StreamNumber, Parameter, (MediaInfo_info_C)InfoKind, (MediaInfo_info_C)SearchKind);};
    //size_t Set (const CString &ToSet, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const CString &OldValue=_T(""))  {MEDIAINFO_TEST_INT; return MediaInfo_SetI (Handle, ToSet, (MediaInfo_stream_C)StreamKind, StreamNumber, Parameter, OldValue);};
    //size_t Set (const CString &ToSet, stream_t StreamKind, size_t StreamNumber, const CString &Parameter, const CString &OldValue=_T(""))  {MEDIAINFO_TEST_INT; return MediaInfo_Set (Handle, ToSet, (MediaInfo_stream_C)StreamKind, StreamNumber, Parameter, OldValue);};
    CString        Option (const CString &Option, const CString &Value=_T(""))  {MEDIAINFO_TEST_STRING; return MediaInfo_Option (Handle, Option, Value);};
    static CString Option_Static (const CString &Option, const CString &Value=_T(""))  {MEDIAINFO_TEST_STRING_STATIC; return MediaInfo_Option (NULL, Option, Value);};
    size_t                  State_Get ()  {MEDIAINFO_TEST_INT; return MediaInfo_State_Get(Handle);};
    size_t                  Count_Get (stream_t StreamKind, size_t StreamNumber=(size_t)-1)  {MEDIAINFO_TEST_INT; return MediaInfo_Count_Get(Handle, (MediaInfo_stream_C)StreamKind, StreamNumber);};

    bool IsReady() {return (Handle && Module)?true:false;}

private :
    void* Handle;
};

class MediaInfoList
{
public :
    MediaInfoList ()                {MediaInfoDLL_Load(); if (!MediaInfoDLL_IsLoaded()) {Handle=NULL; return;}; Handle=MediaInfoList_New();};
    ~MediaInfoList ()               {MEDIAINFO_TEST_VOID; MediaInfoList_Delete(Handle); MediaInfoDLL_UnLoad();};

    //File
    size_t Open (const CString &File, const fileoptions_t Options=FileOption_Nothing) {MEDIAINFO_TEST_INT; return MediaInfoList_Open(Handle, File, (MediaInfo_fileoptions_C)Options);};
    //size_t Open (const unsigned char* Begin, size_t Begin_Size, const unsigned char* End=NULL, size_t End_Size=NULL) {MEDIAINFO_TEST_INT; return MediaInfoList_Open_Buffer(Handle, Begin, Begin_Size, End, End_Size);};
    //size_t Save (size_t FilePos) {MEDIAINFO_TEST_INT; return MediaInfoList_Save(Handle, FilePos);};
    void Close (size_t FilePos=(size_t)-1) {MEDIAINFO_TEST_VOID; return MediaInfoList_Close(Handle, FilePos);};

    //General information
    CString Inform (size_t FilePos=(size_t)-1)  {MEDIAINFO_TEST_STRING; return MediaInfoList_Inform(Handle, FilePos, 0);};
    CString Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t InfoKind=Info_Text)  {MEDIAINFO_TEST_STRING; return MediaInfoList_GetI (Handle, FilePos, (MediaInfo_stream_C)StreamKind, StreamNumber, Parameter, (MediaInfo_info_C)InfoKind);};
    CString Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber, const CString &Parameter, info_t InfoKind=Info_Text, info_t SearchKind=Info_Name)  {MEDIAINFO_TEST_STRING; return MediaInfoList_Get (Handle, FilePos, (MediaInfo_stream_C)StreamKind, StreamNumber, Parameter, (MediaInfo_info_C)InfoKind, (MediaInfo_info_C)SearchKind);};
    //size_t Set (const CString &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const CString &OldValue=_T(""))  {MEDIAINFO_TEST_INT; return MediaInfoList_SetI (Handle, ToSet, FilePos, (MediaInfo_stream_C)StreamKind, StreamNumber, Parameter, OldValue);};
    //size_t Set (const CString &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, const CString &Parameter, const CString &OldValue=_T(""))  {MEDIAINFO_TEST_INT; return MediaInfoList_Set (Handle, ToSet, FilePos, (MediaInfo_stream_C)StreamKind, StreamNumber, Parameter, OldValue);};
    CString        Option (const CString &Option, const CString &Value=_T(""))  {MEDIAINFO_TEST_STRING; return MediaInfoList_Option (Handle, Option, Value);};
    static CString Option_Static (const CString &Option, const CString &Value=_T(""))  {MEDIAINFO_TEST_STRING_STATIC; return MediaInfoList_Option (NULL, Option, Value);};
    size_t        State_Get ()  {MEDIAINFO_TEST_INT; return MediaInfoList_State_Get(Handle);};
    size_t        Count_Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber=(size_t)-1)  {MEDIAINFO_TEST_INT; return MediaInfoList_Count_Get(Handle, FilePos, (MediaInfo_stream_C)StreamKind, StreamNumber);};
    size_t        Count_Get ()  {MEDIAINFO_TEST_INT; return MediaInfoList_Count_Get_Files(Handle);};

    bool IsReady() {return (Handle && Module)?true:false;}

private :
    void* Handle;
};

} //NameSpace
#endif /*__cplusplus*/

#endif
