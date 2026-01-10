#pragma once

#define _CRTDBG_MAP_ALLOC
#define TARGET_MSVC
#define USE_DIRECT3D
#define NO_UNRAR
#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4238) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable : 4244) // conversion from 'x' to 'y', possible loss of data
#pragma warning(disable : 4324) // structure was padded due to __declspec(align())

#include <algorithm>
#include <array>
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
//#include <mdspan>
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

#include <WinSock2.h>

#include <Windows.h>
#include <hstring.h>
#include <restrictederrorinfo.h>
#include <unknwn.h>

// Undefine GetCurrentTime macro to prevent
// conflict with Storyboard::GetCurrentTime
#undef GetCurrentTime

#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Input.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.Networking.Connectivity.h>
#include <winrt/Windows.Networking.Sockets.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.Threading.Core.h>
#include <winrt/Windows.System.UserProfile.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.Core.h>
#include <winrt/Windows.UI.Popups.h>

using namespace winrt;

#include "Debug.h"
#include "FileSys.h"
#include "NeuronHelper.h"

#include "DirectXHelper.h"

using namespace Neuron;

#pragma comment(lib, "OneCore.lib")

#define DARWINIA_VERSION "dev"			// Development
#define DARWINIA_EXE_VERSION 6,0,0,0				// Gold Master
#define STR_DARWINIA_EXE_VERSION "6, 0, 0, 0\0"	// Gold Master

#define APP_NAME        "Multiwinia"
#define APP_NAME_W      L"Multiwinia"

#define APP_VERSION     DARWINIA_VERSION

#define APP_SYSTEM "PC"

#define    DEBUG_RENDER_ENABLED
#define    FLOAT_NUMERICS

#define     DARWINIA_GAMETYPE "multiwinia"
#define     LAN_PLAY_ENABLED
#define		NETWORK_STATS_ENABLED
#define     INCLUDEGAMEMODE_DOMINATION
#define     INCLUDEGAMEMODE_KOTH
#define     INCLUDEGAMEMODE_CTS
#define     INCLUDEGAMEMODE_ROCKET
#define     INCLUDEGAMEMODE_ASSAULT
#define     INCLUDEGAMEMODE_BLITZ
#define	    INCLUDEGAMEMODE_TANKBATTLE
#define     INCLUDE_CRATES_BASIC
#define     INCLUDE_CRATES_ADVANCED
#define		AUTHENTICATION_LEVEL	1
#define     BLOCK_OLD_DARWINIA_OBJECTS

#define DARWINIA_VERSION_STRING DARWINIA_PLATFORM "-" DARWINIA_GAMETYPE "-" DARWINIA_VERSION

#define TEXTURE_EXTENSION "bmp"

#define snprintf _snprintf

// Visual studio 2005 insists that we use the underscored versions
#define stricmp _stricmp
#define strupr _strupr
#define strnicmp _strnicmp
#define strlwr _strlwr

#define itoa _itoa

#define DARWINIA_PLATFORM "win32"

using std::min;
using std::max;

#define HAVE_DSOUND

#define GRACE_TIME 3.0f

#define SAFE_FREE(x)         {free(x);x=NULL;}
#define SAFE_DELETE(x)       {delete x;x=NULL;}
#define SAFE_DELETE_ARRAY(x) {delete[] x;x=NULL;}
#define SAFE_RELEASE(x)      {if(x){(x)->Release();x=NULL;}}

#define GL_DEBUG()			do{ while(int glError = glGetError()){	printf("glError: %d LINE: %d\n", glError, __LINE__); } while(0)
