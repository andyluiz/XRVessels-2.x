#pragma once

#ifdef _WIN32
#include <windows.h>
using XRModuleHandle = HMODULE;
#else
using XRModuleHandle = void *;
#endif
