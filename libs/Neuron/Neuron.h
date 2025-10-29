#pragma once

#define  _CRT_SECURE_NO_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable:4238) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4244) // conversion from 'x' to 'y', possible loss of data
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

#include <algorithm>
#include <array>
#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <exception>
#include <format>
#include <functional>
#include <future>
#include <iterator>
#include <map>
#include <mdspan>
#include <memory>
#include <queue>
#include <ranges>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
// #define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#if !defined WIN32_LEAN_AND_MEAN
// #define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <Windows.h>
#include <hstring.h>
#include <restrictederrorinfo.h>
#include <unknwn.h>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.Threading.Core.h>
#include <winrt/Windows.System.UserProfile.h>
#include <winrt/Windows.System.h>

using namespace winrt;

#include "Debug.h"
#include "DirectXHelper.h"

#pragma comment(lib, "ws2_32.lib")

#define DARWINIA_VERSION "1.5.11"
#define DARWINIA_EXE_VERSION 1, 5, 11, 0
#define STR_DARWINIA_EXE_VERSION "1, 5, 11, 0\0"

#define TARGET_MSVC

#define TARGET_DEBUG

#define DEBUG_RENDER_ENABLED

#ifdef TARGET_DEBUG
#define DARWINIA_GAMETYPE "debug"
#define LOCATION_EDITOR
#define CHEATMENU_ENABLED
#define D3D_DEBUG_INFO
#endif

//#define PROMOTIONAL_BUILD                         // Their company logo is shown on screen

#if !defined(TARGET_DEBUG) &&         \
    !defined(TARGET_FULLGAME) &&      \
    !defined(TARGET_DEMOGAME) &&      \
    !defined(TARGET_DEMO2) &&         \
    !defined(TARGET_PURITYCONTROL) && \
    !defined(TARGET_VISTA) &&         \
    !defined(TARGET_VISTA_DEMO2)
#error "Unknown target, cannot determine game type"
#endif

#ifndef PROFILER_ENABLED
#define DARWINIA_VERSION_PROFILER "-np"
#else
#define DARWINIA_VERSION_PROFILER ""
#endif

#define DARWINIA_VERSION_STRING DARWINIA_PLATFORM "-" DARWINIA_GAMETYPE "-" DARWINIA_VERSION DARWINIA_VERSION_PROFILER

#ifdef TARGET_MSVC

#include <crtdbg.h>
#define snprintf _snprintf

// Visual studio 2005 insists that we use the underscored versions
#include <string.h>
#define stricmp _stricmp
#define strupr _strupr
#define strnicmp _strnicmp
#define strlwr _strlwr
#define strdup _strdup

#include <stdlib.h>
#define itoa _itoa

#define DARWINIA_PLATFORM "win32"

#include <winsock2.h>
#include <windows.h>

#define HAVE_DSOUND
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#define SAFE_FREE(x) \
  {                  \
    free(x);         \
    x = NULL;        \
  }
#define SAFE_DELETE(x) \
  {                    \
    delete x;          \
    x = NULL;          \
  }
#define SAFE_DELETE_ARRAY(x) \
  {                          \
    delete[] x;              \
    x = NULL;                \
  }
#define SAFE_RELEASE(x) \
  {                     \
    if (x) {            \
      (x)->Release();   \
      x = NULL;         \
    }                   \
  }
