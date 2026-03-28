// Server.cpp — Headless dedicated server host for Interstellar Outpost.
//
// This executable is the entry point for the dedicated server. It links
// NeuronCore + NeuronServer and runs without any windowing, DirectX, or
// Windows App SDK dependency.
//
// Current status (Wave 5 complete):
//   - Host structure and tick loop are in place.
//   - NeuronCore systems (preferences, logging) initialize correctly.
//   - server.cpp is decoupled from g_app via IServerHost interface.
//   - NeuronServer include paths reduced to NeuronCore + NeuronClient only.
//
// TODO(migration): Wire up real Server instance:
//   1. #include "server.h" and "IServerHost.h"
//   2. Implement a headless IServerHost (no g_app, no client subsystems).
//   3. Create Server, call SetHost(), Initialise(), then AdvanceIfNecessary()
//      each tick.
//   4. Remove NeuronClient from NeuronServer include paths once
//      servertoclient.h / ftp_manager.h / generic.h are relocated.

// STL headers required by NeuronCore headers (no PCH in this project).
#include <list>
#include <map>
#include <string>

#include "preferences.h"
#include "hi_res_time.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <csignal>

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------

// g_prefsManager is defined in NeuronCore/preferences.cpp; we just use the extern.

static std::atomic<bool> s_running{true};

// ---------------------------------------------------------------------------
// Signal handler — Ctrl+C triggers a clean shutdown.
// ---------------------------------------------------------------------------

static void SignalHandler(int signal)
{
  if (signal == SIGINT || signal == SIGTERM)
  {
    s_running.store(false, std::memory_order_relaxed);
  }
}

// ---------------------------------------------------------------------------
// Server tick loop
// ---------------------------------------------------------------------------

static constexpr const char* kServerVersion = "dev";
static constexpr double kServerTickRate = 0.1; // 100 ms per tick (10 Hz)

static void RunServer()
{
  std::cout << "[server] Entering tick loop (10 Hz). Press Ctrl+C to stop.\n";

  double nextTickTime = GetHighResTime() + kServerTickRate;

  while (s_running.load(std::memory_order_relaxed))
  {
    double now = GetHighResTime();
    if (now >= nextTickTime)
    {
      // TODO(migration): server->AdvanceIfNecessary(now);
      nextTickTime += kServerTickRate;

      // Prevent spiral-of-death if we fall behind.
      if (nextTickTime < now)
        nextTickTime = now + kServerTickRate;
    }

    // Sleep briefly to avoid busy-spinning.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  std::cout << "[server] Tick loop stopped.\n";
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);

  // --test flag: initialise, print status, exit immediately (for CI/smoke tests).
  bool testMode = false;
  for (int i = 1; i < argc; ++i)
  {
    if (std::string_view(argv[i]) == "--test")
      testMode = true;
  }

  std::cout << "[server] Interstellar Outpost Dedicated Server — " << kServerVersion << std::endl;

  // --- Initialise NeuronCore systems ---
  g_prefsManager = new PrefsManager("server_preferences.txt");
  g_prefsManager->Load();
  std::cout << "[server] Preferences loaded." << std::endl;

  // --- Run ---
  if (testMode)
    std::cout << "[server] --test mode: skipping tick loop." << std::endl;
  else
    RunServer();

  // --- Shutdown ---
  std::cout << "[server] Shutting down." << std::endl;
  delete g_prefsManager;
  g_prefsManager = nullptr;

  return 0;
}
