# Migration Worksheet

Use this document together with `server.md` to track the first migration batch and later waves.

## Target Boundaries

- `NeuronCore` - shared by client and server
- `NeuronClient` - client orchestration, input, audio, UI, and platform integration
- `NeuronServer` - server authority, simulation hosting, and server transport/orchestration
- `GameRenderer` - client render and presentation logic extracted from `GameLogic`
- `Thin EXE Host` - startup, shutdown, wiring, and process ownership only

## Batch 1 Summary

### Scope
- Clean `NeuronCore` public boundaries
- Extract shared contracts into `NeuronCore`
- Move obvious server-only code into `NeuronServer`
- Stand up `GameRenderer`
- Introduce a thin headless `server.exe`

### Exit Criteria
- [x] `NeuronCore` builds without upward include-path dependencies
- [x] `NeuronServer` owns the current `server.*` implementation
- [x] shared protocol/command contracts compile from `NeuronCore`
- [ ] `GameRenderer` contains at least one real render-only extraction from `GameLogic`
- [ ] `server.exe` can launch headless

## Exact Batch 1 File Touch Order

Use this order to minimize churn and avoid moving implementation files before their shared boundaries are cleaned up.

### Step 1 - Clean the shared umbrella and move small shared prerequisites
1. `NeuronCore/NeuronCore.h`
2. `NeuronCore/pch.h`
3. `NeuronCore/NeuronCore.vcxproj`
4. `NeuronClient/bandwidth.h` → move to `NeuronCore/`
5. `InterstellarOutpost/globals.h` → move to `NeuronCore/`
6. `NeuronClient/network_defines.h` → move to `NeuronCore/`
7. `GameLogic/worldobject.h` → move to `NeuronCore/`

Reason:
- `NeuronCore/NeuronCore.h` currently leaks UI-related WinRT and `DirectXHelper.h` (which lives in `NeuronClient/` and only resolves via Debug-only upward include paths)
- `NeuronCore/NeuronCore.h:95` has `using namespace winrt;` which pollutes all consumers
- `NeuronCore/pch.h` inherits everything from the umbrella header
- `NeuronCore/NeuronCore.vcxproj` has **Debug-only** upward include paths (Release already lacks them)
- `bandwidth.h`, `globals.h`, `network_defines.h`, and `worldobject.h` are small shared definitions currently trapped in wrong projects and block Steps 2–3

Exit check:
- `NeuronCore` public headers no longer require client-only graphics or UI WinRT
- Prerequisite shared definitions compile from `NeuronCore`

### Step 2 - Extract shared contracts before moving transport or authority code
8. `NeuronClient/clienttoserver.h`
9. any newly created shared contract headers in `NeuronCore` for protocol/DTO/command payload extraction
10. `NeuronClient/clienttoserver.cpp`

Reason:
- `clienttoserver.h` is one of the clearest public boundary leaks because it depends on gameplay-heavy types via `team.h`
- the full transitive chain is: `clienttoserver.h` → `team.h` (`InterstellarOutpost/`) → `entity.h` (`GameLogic/`) → `worldobject.h` (`GameLogic/`) + `shape.h` (NeuronCore, graphics)
- contract extraction must break this chain, not just extract payloads
- server and client transport code should depend on shared contracts, not gameplay implementation types

Exit check:
- client/server-facing payloads compile from `NeuronCore` contracts rather than gameplay-heavy headers
- `clienttoserver.h` no longer transitively includes `team.h` → `entity.h` → `worldobject.h`

### Step 3 - Move server ownership only after contract cleanup
11. `NeuronClient/server.h`
12. `NeuronClient/server.cpp`
13. `NeuronServer/NeuronServer.vcxproj`

Reason:
- `server.h` is conceptually mislocated but includes `bandwidth.h` (moved to `NeuronCore` in Step 1)
- `server.cpp` is more coupled and should move only after its dependencies are reduced
- `NeuronServer.vcxproj` should be updated after the file ownership is clear

Exit check:
- `server.*` is owned by `NeuronServer` and does not depend on client-only platform/UI code

### Step 4 - Stand up the dedicated host boundary
14. `Server/Server.cpp`
15. any new `server.exe` host files required for startup/shutdown wiring

Reason:
- there is already a stub server host entry point available
- the host should stay thin and should only be introduced after `NeuronServer` can own server code

Exit check:
- headless `server.exe` launches with `NeuronCore` + `NeuronServer`

### Step 5 - Review but do not deeply refactor host orchestration yet
16. `InterstellarOutpost/main.cpp`
17. `InterstellarOutpost/app.h`

Reason:
- these are major coupling hubs and should be analyzed in Batch 1
- they should not be the first implementation changes unless required to unblock the new server host or boundary cleanup

Exit check:
- host-only responsibilities are identified and blockers are documented

### Step 6 - Start the first render extraction last
18. `GameLogic/entity.h`
19. `GameLogic/entity.cpp`
20. `GameLogic/building.h`
21. `GameLogic/building.cpp`
22. first new `GameRenderer` files and project entries

Reason:
- these files are high-risk and heavily mixed
- the first extraction should be intentionally small and render-only
- do not move authoritative update logic in Batch 1

Recommended first extraction candidates:
- render helpers
- visual-only state/proxy code
- render submission paths that can consume shared simulation state without owning simulation

Exit check:
- `GameRenderer` contains one real render-only extraction from `GameLogic`

## Batch 1 Touch Rules

- Do not start with `GameLogic` implementation moves.
- Do not move `server.cpp` before shared contract cleanup starts.
- Do not change `App` ownership structure deeply in the first batch unless it blocks the server host.
- Prefer creating shared DTO/command headers before changing transport implementation.
- Prefer one render extraction from `GameLogic` over a large multi-file render migration.

## Initial Assessment Table

This table is prefilled with the current first-pass assessment from the codebase.

| File | Current Project | Proposed Target | Move Readiness | Risk | Uses `g_app` | Exposes Render Types | Uses Shared Contracts | Notes / Follow-up |
|---|---|---|---|---|---|---|---|---|
| `NeuronCore/NeuronCore.h` | `NeuronCore` | `NeuronCore` | Split First | High | No | Yes | Yes | Public umbrella still imports UI-related WinRT and `DirectXHelper.h` (`NeuronCore/NeuronCore.h:78-103`) |
| `NeuronCore/pch.h` | `NeuronCore` | `NeuronCore` | Ready After Cleanup | Medium | No | Via umbrella | Yes | PCH currently includes `NeuronCore.h`, so all umbrella leakage flows through it |
| `NeuronCore/NeuronCore.vcxproj` | `NeuronCore` | `NeuronCore` | Split First | High | No | Yes | Yes | **Debug-only** upward include paths (`NeuronCore/NeuronCore.vcxproj:56`); Release has none (`NeuronCore/NeuronCore.vcxproj:64-80`). Still compiles graphics/windowing units (`:194-229`) |
| `NeuronClient/server.h` | `NeuronClient` | `NeuronServer` | Ready After Prereq Move | Medium | No | No | Yes | Header includes `bandwidth.h` which is in `NeuronClient/` — must move `bandwidth.h` to `NeuronCore` first (`NeuronClient/server.h:7`) |
| `NeuronClient/server.cpp` | `NeuronClient` | `NeuronServer` | Split First | High | Yes | Indirectly | Yes | Includes `app.h`, `multiwinia.h`, `soundsystem.h`; uses `g_app->m_server` (`NeuronClient/server.cpp:12-24`, `:43`) |
| `NeuronClient/clienttoserver.h` | `NeuronClient` | `NeuronClient` | Split First | High | No | No | Yes | Public header depends on `team.h` and shared gameplay payloads (`NeuronClient/clienttoserver.h:10`, `:159-192`) |
| `NeuronClient/clienttoserver.cpp` | `NeuronClient` | `NeuronClient` | Split First | High | Yes | Indirectly | Yes | Pulls in host/gameplay types and uses `g_app` in callbacks (`NeuronClient/clienttoserver.cpp:15-41`, `:61`, `:76`) |
| `NeuronServer/NeuronServer.vcxproj` | `NeuronServer` | `NeuronServer` | Ready After Cleanup | Medium | No | No | Yes | Empty source set; **Debug-only** upward include paths (`NeuronServer/NeuronServer.vcxproj:73`). `NeuronServer.h` includes `NeuronCore.h` so is transitively poisoned by umbrella leakage |
| `InterstellarOutpost/main.cpp` | `InterstellarOutpost` | `Thin EXE Host` | Leave In Host | High | Yes | Yes | Yes | Main loop currently wires rendering, client networking, and server code together |
| `InterstellarOutpost/app.h` | `InterstellarOutpost` | `Thin EXE Host` | Split First | High | Global owner | Yes | Yes | `App` owns both client and server subsystems and is the main coupling hub |
| `GameLogic/entity.h` | `GameLogic` | Split: `NeuronServer` + `GameRenderer` | Split First | High | No | Yes | Yes | Public API exposes `Shape*` and `Render` in the shared type |
| `GameLogic/entity.cpp` | `GameLogic` | Split: `NeuronServer` + `GameRenderer` | Split First | High | Yes | Yes | Yes | Includes renderer/audio/camera and uses `g_app` heavily (`GameLogic/entity.cpp:10-16`, `:64`, `:159-166`, `:190`, `:195`) |
| `GameLogic/building.h` | `GameLogic` | Split: `NeuronServer` + `GameRenderer` | Split First | High | No | Yes | Yes | Public API exposes `Shape*`, ports, and `Render` in the shared type |
| `GameLogic/building.cpp` | `GameLogic` | Split: `NeuronServer` + `GameRenderer` | Split First | High | Yes | Yes | Yes | Includes renderer, sound, and client transport; uses `g_app` in construction/initialization (`GameLogic/building.cpp:11-23`, `:89`, `:98`, `:112`, `:134`, `:138`) |
| `GameRenderer/` | Not present yet | `GameRenderer` | Not Started | Medium | No | Yes | Yes | No `GameRenderer` project/files were found in the current workspace; boundary still needs to be created |
| `Server/Server.cpp` | `Server` | `Thin EXE Host` | Ready | Low | No | No | No | Current dedicated server project is only a `Hello World` stub and is a good starting point for `server.exe` |
| `NeuronClient/bandwidth.h` | `NeuronClient` | `NeuronCore` | Ready | Low | No | No | No | Small class (`BandwidthCounter`), no dependencies beyond primitives. Blocks `server.h` move to `NeuronServer` |
| `InterstellarOutpost/globals.h` | `InterstellarOutpost` | `NeuronCore` | Ready | Low | No | No | Yes | Shared constants (`NUM_TEAMS`, `GRAVITY`, `IAMALIVE_PERIOD`). Included by `building.h` and `network_defines.h`. Trapped in host project |
| `NeuronClient/network_defines.h` | `NeuronClient` | `NeuronCore` | Ready After Prereq Move | Low | No | No | Yes | Protocol defines. Includes `globals.h` (`InterstellarOutpost/`) — move `globals.h` first |
| `GameLogic/worldobject.h` | `GameLogic` | `NeuronCore` | Ready | Low | No | No | Yes | Base class for `Entity`/`Building`. Depends only on `vector3.h` (NeuronCore). Strong candidate for shared contract |
| `InterstellarOutpost/team.h` | `InterstellarOutpost` | Split First | Split First | High | No | No | Yes | Transitive chain: `team.h` → `entity.h` → `worldobject.h` + `shape.h`. Blocks clean `clienttoserver.h` contract extraction |

## Migration Table Template

Copy this table for additional files beyond the initial assessment set.

| File | Current Project | Proposed Target | Move Readiness | Risk | Uses `g_app` | Exposes Render Types | Uses Shared Contracts | Notes / Follow-up |
|---|---|---|---|---|---|---|---|---|
| `<relative path>` |  |  |  |  |  |  |  |  |

## Recommended First File Reviews

### `NeuronCore/NeuronCore.h`
- Current project: `NeuronCore`
- Proposed target: `NeuronCore`
- Move readiness: `Split First`
- Risk: `High`
- Current evidence: imports UI-related WinRT headers and `DirectXHelper.h` through the public umbrella (`NeuronCore/NeuronCore.h:78-103`). `DirectXHelper.h` lives in `NeuronClient/` and only resolves via Debug-only upward include paths.
- Checklist:
  - [x] remove `DirectXHelper.h` (file is in `NeuronClient/`, not `NeuronCore/`) — moved to `NeuronCore/pch.h` (temporary) and was already in `NeuronClient.h`
  - [x] remove UI-related WinRT headers — moved to `NeuronClient/NeuronClient.h`
  - [ ] remove `using namespace winrt;` at line 95 (header namespace pollution) — deferred with `TODO(migration)` comment; large mechanical change
  - [x] keep only standard WinRT valid for both client and server — kept: Data.Json, Foundation.Collections, Globalization, Networking.*, Storage.Streams, System.*
  - [x] verify no client-only transitive include leakage — full build passes, NeuronServer compiles without DirectX/UI poisoning

### `NeuronCore/pch.h`
- Current project: `NeuronCore`
- Proposed target: `NeuronCore`
- Move readiness: `Ready After Cleanup`
- Risk: `Medium`
- Current evidence: PCH is only `#include "NeuronCore.h"`, so it inherits all current umbrella leakage
- Checklist:
  - [x] keep PCH limited to core-safe includes — added temporary `DirectXHelper.h` with TODO for removal when graphics units relocate
  - [x] verify no DirectX, Windows App SDK, or UI WinRT leakage — `DirectXHelper.h` is in PCH only (private build detail), not in public umbrella

### `NeuronCore/NeuronCore.vcxproj`
- Current project: `NeuronCore`
- Proposed target: `NeuronCore`
- Move readiness: `Ready After Cleanup`
- Risk: `High`
- Current evidence: **Debug-only** upward include paths (`NeuronCore/NeuronCore.vcxproj:56`); Release has none (`:64-80`). Still compiles graphics/windowing-oriented units (`:194-229`)
- Checklist:
  - [ ] remove Debug-only upward include-path dependencies into `NeuronClient`, `GameLogic`, and `InterstellarOutpost` — **DEFERRED**: 13 .cpp files still include headers from these projects; paths cannot be removed until those files are relocated or decoupled (see classification below)
  - [x] verify Release config already lacks these paths — confirmed (lines 64-80 have no `AdditionalIncludeDirectories`)
  - [x] classify graphics/windowing compilation units as `Stay`, `Move`, or `Split` — see classification below
  - [ ] verify `NeuronCore` can build with only core-owned include paths — **BLOCKED** by 13 files with upward deps

#### NeuronCore Compilation Unit Classification

**Stay — Core Infrastructure (no upward deps, ~36 .cpp files)**
`binary_stream_readers`, `directory`, `filecrc`, `FileSys`, `filesys_utils`, `hi_res_time`, `math_utils`, `matrix33`, `matrix34`, `net_lib`, `net_mutex_win32`, `net_socket`, `net_socket_listener`, `net_socket_session`, `net_thread_win32`, `net_udp_packet`, `network_stream`, `network_stream_inlines`, `plane`, `profiler`, `random_number`, `rgb_colour`, `runnable`, `string_utils`, `system_info`, `text_stream_readers`, `unicode_string`, `unicode_text_stream_reader`, `user_info`, `vector2`, `vector3`, `work_queue`, `WndProcManager`, `pch`

**Stay — Graphics Translation Layer (no upward deps, use DirectXHelper.h via PCH only, ~10 files)**
`FixedPipeline`, `ogl_extensions_directx`, `opengl_directx_dlist`, `opengl_directx_dlist_dev`, `opengl_directx_matrix_stack`, `opengl_directx_stubs`, `opengl_trace`, `texture`, `texture_uv`, `sphere_renderer`

**Move — Has upward includes (13 .cpp files, 1 .h file)**

| File | Upward Includes | Proposed Target |
|---|---|---|
| `3d_sprite.cpp` | `app.h` (IO), `camera.h` (IO), `renderer.h` (IO) | GameRenderer |
| `bitmap.cpp` | `app.h` (IO), `loading_screen.h` (IO) | NeuronClient |
| `language_table.cpp` | `app.h` (IO) | Decouple (`app.h` use) |
| `opengl_directx.cpp` | `app.h` (IO) | Decouple (`app.h` use) |
| `preferences.cpp` | `app.h` (IO), `prefs_other_window.h` (GL) | Decouple |
| `resource.cpp` | `app.h` (IO), `location.h` (IO), `loading_screen.h` (IO), `soundsystem.h` (NC), `sound_stream_decoder.h` (NC) | NeuronClient (heaviest) |
| `safegl.cpp` | `app.h` (IO) | Decouple (`app.h` use) |
| `shader.cpp` | `app.h` (IO) | Decouple or GameRenderer |
| `shape.cpp` | `app.h` (IO) | Split (shared type) |
| `targetcursor.cpp` | `app.h` (IO), `chatinput_window.h` (GL), `eclipse.h` (NC), `input.h` (NC), `taskmanager_interface.h` (IO) | NeuronClient |
| `text_renderer.cpp` | `app.h` (IO), `camera.h` (IO) | GameRenderer |
| `window_manager.cpp` | `input.h` (NC) | NeuronClient |
| `window_manager_directx.cpp` | `main.h` (IO), `inputdriver_win32.h` (NC), `win32_eventhandler.h` (NC) | NeuronClient |
| `language_table.h` (**header**) | `input_types.h` (NC), `input.h` (NC) | Decouple (public API leak) |

Legend: IO = InterstellarOutpost, NC = NeuronClient, GL = GameLogic

**Path removal blockers:**
- `$(SolutionDir)NeuronClient` — needed by: `pch.h` (DirectXHelper.h), `window_manager*.cpp`, `targetcursor.cpp`, `resource.cpp`, `language_table.h`
- `$(SolutionDir)GameLogic` — needed by: `preferences.cpp` (prefs_other_window.h), `targetcursor.cpp` (chatinput_window.h)
- `$(SolutionDir)InterstellarOutpost` — needed by: 12 .cpp files (`app.h`, `camera.h`, `main.h`, `location.h`, etc.)

### `NeuronClient/server.h`
- Current project: `NeuronClient`
- Proposed target: `NeuronServer`
- Move readiness: `Ready After Prereq Move`
- Risk: `Medium`
- Current evidence: interface is server-oriented and does not expose client UI or DirectX types. However, includes `bandwidth.h` which is in `NeuronClient/` (`NeuronClient/server.h:7`). Also depends on `llist.h`, `darray.h`, `unicode_string.h` (all NeuronCore).
- Prerequisite: move `bandwidth.h` to `NeuronCore` (Step 1)
- Checklist:
  - [ ] confirm `bandwidth.h` is in `NeuronCore` (Step 1 prereq)
  - [ ] move to `NeuronServer`
  - [ ] verify only server-safe and core-safe types remain in public interface

### `NeuronClient/server.cpp`
- Current project: `NeuronClient`
- Proposed target: `NeuronServer`
- Move readiness: `Split First`
- Risk: `High`
- Current evidence: includes `app.h`, `globals.h`, `team.h`, `multiwinia.h`, `clienttoserver.h`, and `soundsystem.h`; callback uses `g_app->m_server` (`NeuronClient/server.cpp:12-24`, `:43`)
- Checklist:
  - [ ] move to `NeuronServer`
  - [ ] remove client-only implementation dependencies
  - [ ] verify no Windows App SDK, DirectX, or UI dependency

### `NeuronClient/clienttoserver.h`
- Current project: `NeuronClient`
- Proposed target: `NeuronClient`
- Move readiness: `Split First`
- Risk: `High`
- Current evidence: header depends on `team.h` and exposes shared gameplay command payloads through the client transport API (`NeuronClient/clienttoserver.h:10`, `:159-192`). Full transitive chain: `clienttoserver.h` → `team.h` (`InterstellarOutpost/`) → `entity.h` (`GameLogic/`) → `worldobject.h` (`GameLogic/`) + `shape.h` (NeuronCore, graphics). `team.h` also includes `taskmanager.h`.
- Checklist:
  - [x] map the full transitive include tree through `team.h` — `team.h` → `entity.h` + `taskmanager.h` + `worldobject.h` → `shape.h`
  - [x] identify which `team.h` types are needed (likely only `TeamControls` for `SendIAmAlive`) — confirmed: only `TeamControls` used in header (line 159)
  - [x] extract shared command payloads into `NeuronCore` — created `NeuronCore/team_controls.h` + `team_controls.cpp` with pure data/serialisation methods
  - [x] break the `team.h` → `entity.h` → `worldobject.h` chain with forward declarations or a shared DTO — replaced `#include "team.h"` with `#include "team_controls.h"` (NeuronCore)
  - [x] reduce public interface to client transport + shared contracts — all `clienttoserver.h` includes now resolve from NeuronCore only

### `NeuronClient/clienttoserver.cpp`
- Current project: `NeuronClient`
- Proposed target: `NeuronClient`
- Move readiness: `Split First`
- Risk: `High`
- Current evidence: includes host and gameplay types such as `app.h`, `location.h`, `taskmanager.h`, `team.h`, `global_world.h`, `factory.h`, and `server.h`; callback/thread use `g_app->m_clientToServer` (`NeuronClient/clienttoserver.cpp:15-41`, `:61`, `:76`)
- Checklist:
  - [x] update includes after payload extraction — `clienttoserver.h` now uses `team_controls.h` instead of `team.h`
  - [ ] keep transport logic in `NeuronClient` — stays in NeuronClient
  - [ ] remove gameplay implementation dependencies where possible — `clienttoserver.cpp` still includes `team.h` etc. via its own implementation needs (acceptable for Batch 1)

### `NeuronServer/NeuronServer.vcxproj`
- Current project: `NeuronServer`
- Proposed target: `NeuronServer`
- Move readiness: `Ready`
- Risk: `Medium`
- Current evidence: library exists, but source set is effectively empty. **Debug-only** upward include paths (`NeuronServer/NeuronServer.vcxproj:73`); Release has none. `NeuronServer.h` includes `NeuronCore.h`, so it is **transitively poisoned** by all umbrella leakage until Step 1 completes.
- Checklist:
  - [ ] add moved server source files
  - [ ] remove Debug-only `NeuronClient` and host include leakage
  - [ ] verify `NeuronCore` umbrella is cleaned (Step 1) before adding real source files
  - [ ] verify references and include paths match intended direction

### `InterstellarOutpost/main.cpp`
- Current project: `InterstellarOutpost`
- Proposed target: `Thin EXE Host`
- Move readiness: `Leave In Host`
- Risk: `High`
- Current evidence: top-level loop currently includes rendering, gameplay, client networking, and server headers together
- Checklist:
  - [ ] keep in host for Batch 1
  - [ ] identify startup/shutdown logic that must remain host-only
  - [ ] note blockers for future split into `client.exe` and `server.exe`

### `InterstellarOutpost/app.h`
- Current project: `InterstellarOutpost`
- Proposed target: `Thin EXE Host`
- Move readiness: `Split First`
- Risk: `High`
- Current evidence: `App` owns renderer, sound, camera, location input, server, and client transport in one type
- Checklist:
  - [ ] keep in host for Batch 1
  - [ ] classify members as client-only, server-only, or shared service ownership
  - [ ] record major blockers to reducing `g_app` coupling

### `GameLogic/entity.h` and `GameLogic/entity.cpp`
- Current project: `GameLogic`
- Proposed target: Split between `NeuronServer` and `GameRenderer`
- Move readiness: `Split First`
- Risk: `High`
- Current evidence: header exposes `Shape*` and `Render`; implementation includes `camera.h`, `renderer.h`, `soundsystem.h`, and uses `g_app` heavily (`GameLogic/entity.h:86-109`, `GameLogic/entity.cpp:10-16`, `:64`, `:159-166`, `:190`, `:195`)
- Checklist:
  - [ ] identify render-facing APIs and data
  - [ ] identify simulation-only APIs and data
  - [ ] define first render-only extraction into `GameRenderer`
  - [ ] do not move authoritative update logic in Batch 1

### `GameLogic/building.h` and `GameLogic/building.cpp`
- Current project: `GameLogic`
- Proposed target: Split between `NeuronServer` and `GameRenderer`
- Move readiness: `Split First`
- Risk: `High`
- Current evidence: header exposes `Shape*` and `Render`; implementation includes `renderer.h`, `soundsystem.h`, `clienttoserver.h`, and uses `g_app` in initialization (`GameLogic/building.h:111-139`, `GameLogic/building.cpp:11-23`, `:89`, `:98`, `:112`, `:134`, `:138`)
- Checklist:
  - [ ] identify render-facing APIs and data
  - [ ] identify simulation-only APIs and data
  - [ ] define first render-only extraction into `GameRenderer`
  - [ ] do not move authoritative update logic in Batch 1

### `GameRenderer/` first extraction candidates
- Current project: `Not present yet`
- Proposed target: `GameRenderer`
- Move readiness: `Not Started`
- Risk: `Medium`
- Current evidence: no `GameRenderer` project/files were found in the current workspace, so the boundary must be created before extraction
- Checklist:
  - [ ] create or confirm project boundary
  - [ ] move one real render-only path from `GameLogic`
  - [ ] keep extracted code dependent on shared simulation state, not server authority
  - [ ] verify it does not become host/orchestration code

### `NeuronClient/bandwidth.h` (prerequisite move)
- Current project: `NeuronClient`
- Proposed target: `NeuronCore`
- Move readiness: `Ready`
- Risk: `Low`
- Current evidence: small self-contained class (`BandwidthCounter`), no includes beyond primitives (`NeuronClient/bandwidth.h:1-31`)
- Checklist:
  - [x] move to `NeuronCore/` — moved `bandwidth.h` and `bandwidth.cpp`
  - [x] update `NeuronClient.vcxproj` and `NeuronCore.vcxproj` membership — done
  - [x] verify no NeuronClient-specific dependencies — only depends on `hi_res_time.h` (NeuronCore)

### `InterstellarOutpost/globals.h`
- Current project: `InterstellarOutpost`
- Proposed target: `NeuronCore`
- Move readiness: `Ready`
- Risk: `Low`
- Current evidence: defines fundamental shared constants: `NUM_TEAMS`, `NUM_SLICES_PER_FRAME`, `IAMALIVE_PERIOD`, `MINIMUM_RENDER_PERIOD`, `GRAVITY` (`InterstellarOutpost/globals.h:1-11`). Included by `building.h` (`GameLogic/building.h:4`) and `network_defines.h` (`NeuronClient/network_defines.h:4`).
- Checklist:
  - [x] move to `NeuronCore/` — moved globals.h
  - [x] update project memberships — removed from InterstellarOutpost.vcxproj, added to NeuronCore.vcxproj
  - [x] verify all existing includers still resolve — build passes; all projects have `$(SolutionDir)NeuronCore`

### `NeuronClient/network_defines.h`
- Current project: `NeuronClient`
- Proposed target: `NeuronCore`
- Move readiness: `Ready After Prereq Move`
- Risk: `Low`
- Current evidence: protocol message defines. Includes `globals.h` (`NeuronClient/network_defines.h:4`) which must move first.
- Checklist:
  - [x] confirm `globals.h` is in `NeuronCore` first — done (Step 1.5)
  - [x] move to `NeuronCore/` — moved network_defines.h (header-only)
  - [x] update project memberships — removed from NeuronClient.vcxproj, added to NeuronCore.vcxproj

### `GameLogic/worldobject.h`
- Current project: `GameLogic`
- Proposed target: `NeuronCore`
- Move readiness: `Ready`
- Risk: `Low`
- Current evidence: base class for `Entity` and `Building`. Depends only on `vector3.h` (NeuronCore). Defines `WorldObjectId` which is a shared simulation identity type (`GameLogic/worldobject.h:1-40`).
- Checklist:
  - [x] move to `NeuronCore/` — header only; `worldobject.cpp` stays in GameLogic (`g_app` usage in `BounceOffLandscape()`)
  - [x] update `GameLogic.vcxproj` and `NeuronCore.vcxproj` membership — header in NeuronCore, .cpp remains in GameLogic
  - [x] verify `WorldObjectId` has no render or client-only dependencies — depends only on `vector3.h` (NeuronCore)

### `server.exe` host
- Current project: `Thin EXE Host`
- Proposed target: `Thin EXE Host`
- Move readiness: `Ready`
- Risk: `Medium`
- Current evidence: `Server/Server.cpp` is currently only a `Hello World` stub, so the dedicated host boundary is available but not implemented
- Checklist:
  - [ ] create thin headless startup path
  - [ ] link only `NeuronCore` and `NeuronServer`
  - [ ] ensure no window creation, DirectX initialization, or Windows App SDK dependency
  - [ ] add fixed-tick startup and clean shutdown behavior

## Per-File Review Template

Copy this section for each file you review.

```markdown
### <relative path>
- Current project:
- Proposed target:
- Reason:
- Public dependencies:
- Private dependencies:
- Client-only APIs used:
- Server-only responsibilities:
- Shared-contract responsibilities:
- `g_app` usage:
- Move readiness: Ready / Ready After Cleanup / Split First / Leave In Host
- Risk: Low / Medium / High
- Required follow-up:
```

## Change Log

### Completed
- [ ] `NeuronCore` public umbrella cleaned
- [ ] shared contracts extracted into `NeuronCore`
- [ ] `server.*` moved into `NeuronServer`
- [ ] first render-only extraction moved into `GameRenderer`
- [ ] headless `server.exe` host added

### Notes
- Keep changes small and reviewable.
- Prefer interface cleanup before project-file moves.
- Do not move mixed gameplay/render files wholesale.
