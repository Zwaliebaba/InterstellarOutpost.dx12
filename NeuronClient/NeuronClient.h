#pragma once

#define CHEATMENU_ENABLED

#include "NeuronCore.h"

// Client-only WinRT headers (removed from NeuronCore.h during migration Step 1)
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Input.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.Core.h>
#include <winrt/Windows.UI.Popups.h>

#include "DirectXHelper.h"
