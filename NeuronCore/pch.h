#pragma once

#include "NeuronCore.h"

// TODO(migration): Remove once graphics/windowing compilation units are relocated out of NeuronCore.
// This keeps DirectX types available to NeuronCore's internal .cpp files without polluting the public umbrella.
#include "DirectXHelper.h"
