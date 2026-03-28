// ServerStubs.cpp — Linker stubs for the dedicated server host.
//
// Server.exe links NeuronCore.lib + NeuronServer.lib only (no NeuronClient).
// Some NeuronCore object files reference symbols whose implementations live in
// NeuronClient's OpenGL→DirectX layer. Stubs are provided here so the linker
// is satisfied; they are either no-ops or behind runtime guards that are never
// reached on a headless server.
//
// Wave 5 status:
//   - g_app: REMOVED — server.cpp no longer references g_app (decoupled via IServerHost).
//   - glFinish: still needed (profiler.cpp forward-declares it).
//
// TODO(migration): Delete this file after profiler.cpp glFinish() dependency
// is removed or replaced with a callback.

// ===========================================================================
// Function stubs — symbols referenced by NeuronCore.lib obj files.
// ===========================================================================

// profiler.cpp calls glFinish() behind a runtime guard (m_doGlFinish).
// The server profiler never enables GPU finish, so this is safe as a no-op.
void glFinish() {}
