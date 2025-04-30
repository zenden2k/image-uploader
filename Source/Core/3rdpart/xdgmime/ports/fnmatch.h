#pragma once

#define FNM_PATHNAME (1 << 0) 
#define FNM_NOESCAPE (1 << 1) 
#define FNM_PERIOD (1 << 2)

#define FNM_FILE_NAME (1 << 0)
#define FNM_LEADING_DIR (1 << 3) 
#define FNM_CASEFOLD (1 << 4) 

#define FNM_NOMATCH 1

#ifdef __cplusplus
extern "C" {
#endif

int fnmatch(const char* pattern, const char* string, int flags);

#ifdef __cplusplus
}
#endif
