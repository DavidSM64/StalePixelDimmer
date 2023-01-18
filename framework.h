// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


#if 1 //def _DEBUG
extern char printBuffer[256];
#define printMsg(...) \
    sprintf_s(printBuffer, 256, __VA_ARGS__); \
    OutputDebugStringA((LPCSTR)printBuffer);
#else
#define printMsg(...) 
#endif