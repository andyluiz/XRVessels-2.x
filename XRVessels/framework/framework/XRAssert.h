#pragma once

#ifdef _WIN32
#include <crtdbg.h>
#else
#include <cassert>
#define _ASSERTE(condition) assert(condition)
#endif
