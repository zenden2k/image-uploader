#pragma once

#include <stdio.h>
#include <sys/stat.h>

FILE* fopen_utf8(char const* fileName, char const* mode);
int stat_utf8(char const* const _FileName, struct stat* const _Stat);

