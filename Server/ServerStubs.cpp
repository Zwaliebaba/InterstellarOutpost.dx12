// ServerStubs.cpp — Temporary linker stubs for the dedicated server host.
//
// NeuronCore.lib's graphics/windowing files have been relocated to NeuronClient,
// so most of the original stubs are no longer needed. Remaining stubs:
//   - g_app: NeuronServer's server.cpp still uses g_app (~10 call sites).
//   - glFinish: profiler.cpp (NeuronCore) forward-declares glFinish(); the
//     implementation lives in NeuronClient's OpenGL→DirectX layer.
//
// TODO(migration): Delete this file after:
//   1. server.cpp g_app coupling is removed (Wave 5).
//   2. profiler.cpp glFinish() dependency is removed or replaced with a callback.

#include <cstdlib>
#include <cstdio>

// ---------------------------------------------------------------------------
// Crash helper — called if a stub is reached at runtime.
// ---------------------------------------------------------------------------

[[noreturn]] static void StubFatal(const char* name)
{
  fprintf(stderr, "[server] FATAL: stub reached — %s\n", name);
  std::abort();
}

// ===========================================================================
// Global pointers required by NeuronServer.lib obj files (server.cpp).
// ===========================================================================

class App;
App* g_app = nullptr;

// ===========================================================================
// Function stubs — symbols referenced by NeuronCore.lib obj files.
// ===========================================================================

// profiler.cpp calls glFinish() behind a runtime guard (m_doGlFinish).
// The server profiler never enables GPU finish, so this is safe as a no-op.
void glFinish() {}
