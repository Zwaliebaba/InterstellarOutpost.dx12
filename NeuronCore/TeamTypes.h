// TeamTypes.h — Shared team type constants used by both client and server.
// Extracted from team.h (Wave 5) so NeuronServer can reference team types
// without pulling in the full Team class and its GameLogic dependencies.

#pragma once

enum
{
  TeamTypeUnused = -1,
  TeamTypeLocalPlayer,
  TeamTypeRemotePlayer,
  TeamTypeCPU,
  TeamTypeSpectator,
};
