#pragma once

#include "NeuronClient.h"

static const UINT FrameCount = 3;

static const UINT NumContexts = 3;
static const UINT NumLights = 3;        // Keep this in sync with "shaders.hlsl".

// Command list submissions from main thread.
static const int CommandListCount = 3;
static const int CommandListPre = 0;
static const int CommandListMid = 1;
static const int CommandListPost = 2;
