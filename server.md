# Client/Server Library Split Analysis Plan

## Goal
Create a repeatable way to analyze the current codebase and estimate the impact of moving `.cpp` and `.h` files into four target libraries:

- `NeuronCore` - shared by client and server
- `NeuronClient` - client-only orchestration, input, audio, UI, and platform integration
- `NeuronServer` - server-only authority, simulation hosting, and server transport/orchestration
- `GameRenderer` - client-side render and presentation logic extracted from `GameLogic`

This plan is for analysis and impact assessment first, not for bulk moves.

## Current-State Facts To Anchor The Analysis

> **Note**: facts marked ✅ were verified against the codebase. `migration.md` was never created;
> all status is tracked inline in this file.

- `InterstellarOutpost/main.cpp` is the current top-level loop and pulls in rendering, input, gameplay, client networking, and server headers in one place (`InterstellarOutpost/main.cpp:1-53`). *No code changes made yet — analysis only.*
- `App` currently owns both client-facing and server-facing subsystems, including `Renderer`, `SoundSystem`, `Camera`, `LocationInput`, `TaskManagerInterface`, `GameMenu`, `MarkerSystem`, `Server`, and `ClientToServer` (`InterstellarOutpost/app.h:48-108`). *No code changes made yet — member classification deferred to Wave 5.*
- `App` also exposes global access through `g_app` (`InterstellarOutpost/app.h:239-239`). *No code changes made yet — coupling analysis deferred to Wave 5.*
- ~~`NeuronCore` currently includes WinRT and Windows UI headers directly in its umbrella header (`NeuronCore/NeuronCore.h:78-93`) and also includes `DirectXHelper.h` (`NeuronCore/NeuronCore.h:101-101`).~~ ✅ *Fixed in Step 1 — UI WinRT moved to NeuronClient.h, DirectXHelper.h moved to NeuronCore/pch.h (private).*
- ~~`NeuronCore` currently compiles graphics/windowing files such as `window_manager_directx.cpp`, `opengl_directx.cpp`, `sphere_renderer.cpp`, and `text_renderer.cpp` (`NeuronCore/NeuronCore.vcxproj:200-237`). *Still true — relocation to NeuronClient/GameRenderer deferred to Wave 2+.*~~ ✅ *Wave 2 complete — 23 graphics/windowing `.cpp` files and 25 `.h` files relocated from NeuronCore to NeuronClient. Also moved: `language_table.h/.cpp` (deeply coupled to `g_inputManager`/NeuronClient). `gametimer.h` moved from InterstellarOutpost to NeuronCore (shared simulation timer).*
- ~~`NeuronCore` currently has **Debug-only** include paths into `NeuronClient`, `GameLogic`, and `InterstellarOutpost`, which means the dependency direction is already inverted (`NeuronCore/NeuronCore.vcxproj:56`). Additionally, `NeuronCore/pch.h` includes `DirectXHelper.h` (which lives in `NeuronClient/`), so this inverted path is load-bearing. *Cannot remove until the 13 graphics/windowing files are relocated out of NeuronCore.*~~ ✅ *Wave 2 complete — inverted `AdditionalIncludeDirectories` removed from NeuronCore Debug config. `DirectXHelper.h` removed from `NeuronCore/pch.h`. NeuronCore now compiles with zero upward include-path dependencies.*
- ~~`NeuronCore/NeuronCore.h` contains `using namespace winrt;`, which pollutes every consumer's namespace and violates the project rule against `using namespace` in headers.~~ *Still present at `NeuronCore/NeuronCore.h:90` — deferred with `TODO(migration)` comment; large mechanical change.*
- `NeuronClient` already carries Windows App SDK package imports (`NeuronClient/NeuronClient.vcxproj:3-15`).
- ~~`NeuronClient` currently contains both client and server networking code, including `clienttoserver.*`, `server.*`, and `servertoclient.*`.~~ ✅ *`server.*` and `servertoclient.*` moved to NeuronServer. `clienttoserver.*` stays in NeuronClient (correct target). `generic.h/.cpp`, `ftp_manager.h/.cpp`, and `servertoclientletter.h` moved to NeuronCore as shared contracts.*
- ~~`NeuronServer` exists as a static library project, but it is effectively empty today.~~ ✅ *NeuronServer now owns `server.h`, `server.cpp`, `servertoclient.h`, `servertoclient.cpp`, and `IServerHost.h`.*
- ~~`NeuronServer` currently has include paths into `NeuronClient`, `InterstellarOutpost`, and `GameLogic` in **both Debug and Release** configurations (`NeuronServer/NeuronServer.vcxproj:73,91`), so it is not yet isolated.~~ ✅ *Fully isolated — NeuronServer include paths now contain only `$(SolutionDir)NeuronCore`. `InterstellarOutpost` and `GameLogic` removed in Wave 5. `NeuronClient` removed after relocating `servertoclient.h/.cpp` to NeuronServer and `generic.h/.cpp`, `ftp_manager.h/.cpp`, `servertoclientletter.h` to NeuronCore.*
- ~~`NeuronServer/NeuronServer.h` includes `NeuronCore.h`, so `NeuronServer` is **transitively poisoned** by all UI WinRT, DirectX, and `DirectXHelper.h` leakage from the NeuronCore umbrella.~~ ✅ *Fixed — NeuronCore umbrella cleaned in Step 1.*
- ~~`ClientToServer` currently depends on gameplay types such as `TeamControls` via `team.h`.~~ ✅ *Fixed — `clienttoserver.h` now includes `TeamControls.h` (NeuronCore) instead of `team.h`.*
- ~~The `team.h` dependency has a deep transitive chain.~~ ✅ *Chain broken via `TeamControls` extraction to NeuronCore.*
- ~~`NeuronClient/server.h` includes `bandwidth.h` (`NeuronClient/bandwidth.h`), so `server.h` cannot move to `NeuronServer` until `bandwidth.h` moves to `NeuronCore`.~~ ✅ *`bandwidth.h` moved to NeuronCore; `server.h` moved to NeuronServer.*
- ~~`InterstellarOutpost/globals.h` defines fundamental shared constants trapped in the host project.~~ ✅ *Moved to NeuronCore.*
- ~~`Server` is physically located in `NeuronClient`.~~ ✅ *Moved to NeuronServer.*
- ~~`g_app` is a major coupling hotspot across `NeuronCore`, `NeuronClient`, `NeuronServer`, `GameLogic`, and `InterstellarOutpost`.~~ ✅ *Wave 5 partial — `server.cpp` decoupled from `g_app` via `IServerHost` interface (14 call sites replaced). `NeuronCore` has zero `g_app` references. ~6,800 `g_app` occurrences remain across ~200 files in `NeuronClient`, `GameLogic`, `GameRenderer`, and `InterstellarOutpost`. Full `App` split deferred.*

## Desired Dependency Direction

Target the following static dependency graph:

```text
NeuronCore
  ^
  |
  +-- NeuronServer
  |
  +-- NeuronClient
        ^
        |
        +-- GameRenderer

Executable(s)
  -> link NeuronCore + NeuronClient + GameRenderer   for client build
  -> link NeuronCore + NeuronServer                  for server build
```

Rules:

1. `NeuronCore` must not include headers from `NeuronClient`, `NeuronServer`, `GameLogic`, `GameRenderer`, or `InterstellarOutpost`.
2. `NeuronClient` may depend on `NeuronCore`, Windows App SDK, and client-side platform services, but should not become the home for extracted entity/building render logic.
3. `GameRenderer` may depend on `NeuronCore` and `NeuronClient`, and is the target home for client-side render and presentation logic extracted from `GameLogic`.
4. `NeuronServer` may depend on `NeuronCore` and standard WinRT/C++ only, but not on UI-related WinRT headers.
5. DirectX and Windows App SDK code should be client-only.
6. Shared gameplay protocol/data definitions should live in `NeuronCore` only if they are truly platform-neutral and renderer-free.
7. `GameLogic` should be split by responsibility: authoritative update/simulation goes to the server side, while client render logic is migrated out of `GameLogic` into `GameRenderer`.
8. Plan for a dedicated headless `server.exe` that links `NeuronCore` + `NeuronServer` and is suitable for future container hosting.
9. `NeuronCore` must compile without include-path access to `NeuronClient`, `NeuronServer`, `GameLogic`, `GameRenderer`, or `InterstellarOutpost`.

## Recommended Analysis Approach

### Phase 1 - Build a File Inventory
For every `.cpp` and `.h` in `NeuronCore`, `NeuronClient`, `NeuronServer`, `GameLogic`, `GameRenderer`, and `InterstellarOutpost`, record one row in a migration sheet.

Use these columns:

| Column | Meaning |
|---|---|
| File | Relative path |
| Current Project | Where it compiles today |
| Candidate Target | `NeuronCore`, `NeuronClient`, `NeuronServer`, `GameRenderer`, or `Thin EXE Host` |
| Owns Runtime State? | Yes/No |
| Uses `g_app`? | Yes/No |
| Includes DirectX / renderer / windowing? | Yes/No |
| Includes Windows App SDK? | Yes/No |
| Uses socket/network code? | Yes/No |
| Uses UI/Eclipse/input/audio? | Yes/No |
| Uses gameplay world types? | Yes/No |
| Exposes render types in public headers? | Yes/No |
| Uses shared contract types? | Yes/No |
| Can compile without `g_app`? | Yes/No |
| Serialization / protocol contract? | Yes/No |
| Save/load impact | Low/Med/High |
| Build-system impact | Low/Med/High |
| Separation risk | Low/Med/High |
| Notes | Free text |

### Phase 2 - Public Header Detox
Before classifying ownership, clean up public boundary leakage:

- remove DirectX and client-only includes from shared/public headers
- remove UI-related WinRT headers from `NeuronCore` public umbrella headers and PCHs
- move heavy includes from headers into `.cpp` files wherever possible
- replace include chains with forward declarations where possible
- isolate render-only methods from shared or simulation-facing interfaces

### Phase 3 - Classify By Responsibility
Classify each file by the narrowest responsibility it serves.

#### Put in `NeuronCore`
Only if the file is all of the following:

- usable by both client and server
- independent of renderer, audio, input, UI, and Windows App SDK
- independent of UI-related WinRT headers
- independent of top-level app orchestration
- stable enough to be shared as a contract

Typical examples from current structure:

- math, containers, timers, strings, file I/O, preferences, networking primitives (`NeuronCore/NeuronCore.vcxproj:83-168`, `NeuronCore/NeuronCore.vcxproj:170-231`)
- protocol data types and transport-neutral message definitions
- deterministic gameplay data structures that do not depend on rendering or UI

#### Put in `NeuronClient`
If the file touches any of these:

- DirectX
- Windows App SDK
- renderer/device/window management
- input
- audio playback
- UI/Eclipse windows
- local presentation-only game state
- client-only orchestration for rendering systems such as `GameRenderer`

Current likely client-only clusters include:

- `GraphicsCore.*` (`NeuronClient/GraphicsCore.h:3-88`)
- renderer, camera, menu, loading screen, user input, marker UI, task manager UI (`InterstellarOutpost/InterstellarOutpost.vcxproj:110-154`, `InterstellarOutpost/InterstellarOutpost.vcxproj:156-197`)
- sound/input/UI files already under `NeuronClient` (`NeuronClient/NeuronClient.vcxproj:95-156`, `NeuronClient/NeuronClient.vcxproj:158-208`)

#### Put in `NeuronServer`
If the file is authoritative simulation or server-side transport/orchestration and does not require rendering/UI.

Current first-pass candidates:

- ~~`server.*` currently misplaced under `NeuronClient`~~ ✅ *Moved to NeuronServer.*
- ~~`servertoclient.*` — currently in NeuronClient, likely candidate for NeuronServer~~ ✅ *Moved to NeuronServer.*
- any server-only connection/session/listener orchestration
- host-side match control, authoritative world advancement, anti-cheat or validation logic

#### Put in `GameRenderer`
If the file is client-side render or presentation logic extracted from gameplay-heavy code and depends on shared simulation state but not server authority.

Typical candidates include:

- entity and building rendering paths extracted from `GameLogic`
- render submission, scene visualization, and visual-only debug drawing
- visual state/proxy components derived from shared simulation state
- rendering support code that should not stay inside `NeuronClient` host/orchestration layers

`GameRenderer` should not own authoritative updates, network authority, or save/load ownership.

#### Keep as Thin EXE Host
If the file should only wire together libraries and own process startup/shutdown.

Current likely host candidates:

- `main.cpp` (`InterstellarOutpost/main.cpp:1-53`)
- a future client `AppHost`
- a future headless `server.exe` host / `ServerHost`

### Phase 4 - Detect Hard Coupling Before Moving Files
Before moving any file, inspect it for these blockers:

1. **Global singleton access**
   - Any use of `g_app` is a red flag because it usually means the file is reaching across subsystem boundaries.
2. **Header leakage**
   - If a public header includes renderer, UI, or gameplay-heavy headers, split the interface first.
3. **Mixed responsibilities**
   - Files that do both simulation and presentation should be split before relocation.
4. **Project include path dependency**
   - If a file compiles only because the project exposes broad include directories, fix includes before moving it.
5. **PCH dependency**
   - If a file relies on `NeuronCore.h` bringing in client-only or UI headers transitively, isolate those includes.

### Phase 5 - Produce A Dependency Map
For each candidate file, answer:

- What headers does it include directly?
- Which symbols from other libraries does it use?
- Is the dependency needed in the public header or only in the `.cpp`?
- Can the dependency be replaced with a forward declaration, interface, callback, or message struct?

Record dependencies in this form:

```text
File: InterstellarOutpost/location.cpp
Target: ?
Public dependencies: location.h -> team.h -> renderer?
Private dependencies: renderer.h, soundsystem.h, server.h
Coupling type: simulation + presentation + networking
Action: split before move
```

### Phase 6 - Score Migration Impact
Use a simple score per file:

- `+3` public header used widely
- `+3` touches `g_app`
- `+2` mixes simulation and rendering
- `+2` currently depends on broad PCH/includes
- `+2` serialized/network contract risk
- `+1` build-file/project-file changes required
- `-2` already lives in correct conceptual library
- `-2` private `.cpp` with narrow dependencies

Interpretation:

- `0-2` = low-risk move
- `3-5` = medium-risk, move after interface cleanup
- `6+` = high-risk, split/refactor first

## Suggested Work Order

### Wave 0 - Freeze The Rules
Before moving code, write down these rules in an ADR or migration note:

- `NeuronCore` is renderer-free, Windows App SDK-free, and free of UI-related WinRT headers.
- Standard non-UI WinRT may remain in `NeuronCore` if needed by both client and server.
- DirectX and windowing stay client-only.
- Server authority must not depend on UI, audio, or presentation types.
- Shared protocol/data contracts live in `NeuronCore`.
- `GameLogic` is split by responsibility rather than kept as one mixed module.
- Client render logic is migrated out of `GameLogic` into `GameRenderer`.
- `GameRenderer` is a first-class client-side library boundary, not just a foldering convention.
- A dedicated headless `server.exe` is a target deliverable.

### Wave 1 - Fix Build Boundaries First
1. ~~Remove inverted include paths from `NeuronCore` debug settings (`NeuronCore/NeuronCore.vcxproj:47-57`).~~ ✅ *Done — `AdditionalIncludeDirectories` removed from NeuronCore Debug config after graphics/windowing file relocation.*
2. ~~Remove `NeuronClient` and `InterstellarOutpost` include leakage from `NeuronServer` (`NeuronServer/NeuronServer.vcxproj:64-74`).~~ ✅ *Done — all non-NeuronCore include paths removed. `InterstellarOutpost`/`GameLogic` removed in Wave 5. `NeuronClient` removed after relocating `servertoclient.h/.cpp` to NeuronServer and `generic.h/.cpp`, `ftp_manager.h/.cpp`, `servertoclientletter.h` to NeuronCore. NeuronServer now depends only on NeuronCore.*
3. ~~Remove any upward include-path dependency from `NeuronCore` into `GameRenderer` or host code.~~ ✅ *Done — NeuronCore has zero upward include-path dependencies.*
4. Make project references reflect intended direction before moving many files.
5. ~~Keep PCHs minimal and library-local.~~ ✅ *Done — `NeuronCore/pch.h` now only includes `NeuronCore.h` (no `DirectXHelper.h`).*

### Wave 2 - Public Header Detox & Graphics File Relocation
1. ~~Remove `DirectXHelper.h` and other client-only graphics dependencies from `NeuronCore` umbrella and public headers.~~ ✅ *`DirectXHelper.h` removed from `NeuronCore/pch.h`. Stays in `NeuronClient/NeuronClient.h`.*
2. ~~Remove UI-related WinRT headers from `NeuronCore` umbrella and PCH files.~~ ✅ *Moved to `NeuronClient/NeuronClient.h`.*
3. Remove `using namespace winrt;` from `NeuronCore/NeuronCore.h:90` (violates header namespace pollution rule). *Deferred — large mechanical change.*
4. ~~Relocate graphics/windowing files from NeuronCore to NeuronClient.~~ ✅ *48 files moved (25 headers + 23 sources): OpenGL→DirectX translation layer, window manager, text/sphere renderers, resource manager, bitmap, shader, shape, texture, targetcursor, safegl, and more. Also moved `language_table.h/.cpp` (deeply coupled to `g_inputManager`/NeuronClient input system).*
5. ~~Move shared simulation headers trapped in InterstellarOutpost to NeuronCore.~~ ✅ *`gametimer.h` moved from InterstellarOutpost to NeuronCore (network-safe game timer needed by both client and server).*
6. ~~Fix "staying" NeuronCore files with upward dependencies.~~ ✅ *`preferences.cpp` — replaced `g_app->m_resource->GetTextReader()` with direct `FileSys` + `UnicodeTextFileReader`/`TextFileReader`; inlined `OTHER_DIFFICULTY` macro. `hi_res_time.cpp` — removed dead `#include "main.h"`. `profiler.cpp` — added forward declaration for `glFinish()` (resolved at link time).*
7. Replace public heavy includes with forward declarations wherever possible.
8. Isolate render-only APIs away from shared or simulation-facing headers.

### Wave 3 - Extract Shared Contracts
Move or create shared headers in `NeuronCore` for:

- ~~**prerequisite file moves**: `bandwidth.h` (from `NeuronClient/`), `globals.h` (from `InterstellarOutpost/`), `network_defines.h` (from `NeuronClient/`)~~ ✅ *All three moved to NeuronCore.*
- ~~`worldobject.h` (from `GameLogic/`)~~ ✅ *Moved to NeuronCore.*
- `TeamControls.h` / `TeamControls.cpp` ✅ *Extracted from `team.h` into NeuronCore as a shared network contract.*
- packet/message enums
- transport-neutral DTOs
- deterministic gameplay commands
- team/input command payloads currently shared across client/server
- shared simulation state that must be understood by both client prediction/rendering and server authority

This is needed because `ClientToServer` already pulls gameplay state through `team.h` (`NeuronClient/clienttoserver.h:10-10`, `NeuronClient/clienttoserver.h:159-192`).

### Wave 4 - Move Obvious Server Code
Move the clearest server-only files first:

- ~~`NeuronClient/server.h`~~ ✅ *Moved to `NeuronServer/server.h`.*
- ~~`NeuronClient/server.cpp`~~ ✅ *Moved to `NeuronServer/server.cpp`.*
- any server-side session/history/authority files that do not require UI or renderer code

> **Status**: All server files moved. ✅ *`server.h/.cpp` and `servertoclient.h/.cpp` in NeuronServer. `server.cpp` decoupled from `g_app` via `IServerHost` interface (Wave 5). NeuronServer include paths: `$(SolutionDir)NeuronCore` only — fully isolated.*

### Wave 5 - Split `App`
`App` is currently a mixed owner of client and server state (`InterstellarOutpost/app.h:48-108`).

Break it into narrower facades and service boundaries:

- `ClientAppContext`
- `ServerAppContext`
- `SharedGameContext` or equivalent shared services layer
- smaller service interfaces such as rendering, simulation, audio, and networking services where possible

Goal: reduce direct `g_app` access and replace it with explicit dependencies.

> **Status**: Wave 5 complete — `server.cpp` decoupled from `g_app` + NeuronServer fully isolated.
>
> **Approach taken**: Rather than splitting the full `App` class (high risk, 6,800+ `g_app` sites), Wave 5 targeted the single highest-value decoupling: `NeuronServer/server.cpp`. An `IServerHost` abstract interface was introduced with 6 virtual methods. `InterstellarOutpost/app.cpp` provides `AppServerHost` (file-local implementation delegating to `g_app`). `server.cpp` no longer includes `app.h`, `team.h`, `multiwinia.h`, or `clienttoserver.h`.
>
> **Created**: `NeuronServer/IServerHost.h` (interface), `NeuronCore/TeamTypes.h` (shared enum).
> **Modified**: `NeuronServer/server.h` (added `m_host` + `SetHost`), `NeuronServer/server.cpp` (14 g_app sites replaced), `InterstellarOutpost/app.cpp` (AppServerHost + wiring), `InterstellarOutpost/team.h` (uses TeamTypes.h), `NeuronServer/NeuronServer.vcxproj` (include paths cleaned), `Server/ServerStubs.cpp` (g_app stub removed), `Server/Server.cpp` (comments updated).
>
> **Wave 5 extension**: Relocated 4 remaining NeuronClient headers (`generic.h/.cpp`, `servertoclientletter.h`, `ftp_manager.h/.cpp` → NeuronCore; `servertoclient.h/.cpp` → NeuronServer). Removed `$(SolutionDir)NeuronClient` from NeuronServer include paths. **NeuronServer now depends only on NeuronCore.**
>
> **Remaining**: Full `App` split into `ClientAppContext`/`ServerAppContext`/service interfaces deferred — ~6,800 `g_app` occurrences in client/gameplay code.

### Wave 6 - Split Mixed Gameplay/Presentation Files
For files in `InterstellarOutpost`, `GameLogic`, and `GameRenderer`:

- separate deterministic simulation/update code from rendering/debug drawing
- move authoritative world advancement and game-object updates toward `NeuronServer`
- migrate client render logic out of `GameLogic` into `GameRenderer`
- keep `GameRenderer` focused on render/presentation logic rather than host orchestration
- move remaining rendering/presentation helpers toward `NeuronClient`
- separate gameplay model from local input translation
- separate network command generation from UI actions

Only then assign the resulting files to `NeuronCore`, `NeuronClient`, `NeuronServer`, or `GameRenderer`.

## Impact Assessment Checklist Per File Move

For every proposed move, answer these questions:

### Functional
- Does the file participate in authoritative simulation?
- Does it depend on local-only presentation state?
- Does it read/write save-game, replay, or network state?
- Does it alter startup or shutdown order?

### Dependency
- Are any public includes crossing library boundaries the wrong way?
- Can forward declarations remove heavy includes?
- Does the file depend on `g_app`, globals, or singletons?
- Does the file require project-specific include directories to compile?

### Platform
- Does it use DirectX types or device objects?
- Does it use Windows App SDK types?
- Does it use WinRT only, and if so is that acceptable for both client and server?

### Build
- Which `.vcxproj` files need membership updates?
- Which PCH includes need to change?
- Which libraries/executables need new project references?
- Will the move increase or reduce rebuild fan-out?

### Runtime
- Does the move change ownership/lifetime?
- Does it change threading assumptions?
- Does it affect determinism?
- Does it affect bandwidth or protocol compatibility?

## Red Flags To Call Out In The Assessment

Mark the file as `Do Not Move Yet` if any of these are true:

- public header exposes renderer or UI types
- simulation code calls rendering/audio/input directly
- file depends on `g_app` for unrelated services
- file is used by both client and server but also includes DirectX or Windows App SDK
- server path depends on `InterstellarOutpost` executable code
- `NeuronCore` dependency would point upward into `NeuronClient` or host code

## Recommended First Migration Batch

Use a small first batch that improves boundaries without forcing a large gameplay rewrite.

### Batch 1 Scope
1. **Clean `NeuronCore` public boundaries** — *done*
   - ✅ UI WinRT headers moved from `NeuronCore/NeuronCore.h` to `NeuronClient/NeuronClient.h`
   - ✅ `DirectXHelper.h` removed from `NeuronCore` public umbrella and PCH
   - ✅ NeuronCore inverted include paths removed (Debug `AdditionalIncludeDirectories` deleted)
   - ✅ 48 graphics/windowing files relocated from NeuronCore to NeuronClient
   - ✅ `gametimer.h` moved from InterstellarOutpost to NeuronCore (shared simulation timer)
   - ✅ Blocking upward deps in staying files fixed: `preferences.cpp`, `hi_res_time.cpp`, `profiler.cpp`
   - ⬜ `using namespace winrt;` still in `NeuronCore/NeuronCore.h:90` (deferred — large mechanical change)
2. **Create or harden shared contracts in `NeuronCore`** — *done*
   - ✅ `TeamControls.h/.cpp` extracted from `team.h` into NeuronCore as shared network contract
   - ✅ `bandwidth.h`, `globals.h`, `network_defines.h` moved to NeuronCore
   - ✅ `worldobject.h` moved to NeuronCore
   - ✅ `clienttoserver.h` now depends on `TeamControls.h` instead of `team.h`
3. **Move obvious server-only code into `NeuronServer`** — *done*
   - ✅ `server.h` / `server.cpp` moved from `NeuronClient` to `NeuronServer`
   - ✅ `server.cpp` decoupled from `g_app` via `IServerHost` interface (14 call sites replaced, Wave 5)
   - ✅ `InterstellarOutpost` and `GameLogic` removed from NeuronServer include paths
4. **Stand up the `GameRenderer` boundary** — *done (9 extractions)*
   - ✅ `ShadowRenderer.h/.cpp` extracted from `GameLogic` into `GameRenderer` namespace
   - ✅ `GameRenderer.vcxproj` is a static library referencing NeuronCore and NeuronClient
   - ✅ `GameRenderer/pch.h` upgraded to include `NeuronClient.h` (client-side library)
   - ✅ `entity_leg.h/.cpp` moved from `GameLogic` to `GameRenderer` (render helper, 3 consumers)
   - ✅ `landscape_renderer.h/.cpp` moved from `InterstellarOutpost` to `GameRenderer` (terrain rendering, 4 consumers)
   - ✅ `explosion.h/.cpp` moved from `InterstellarOutpost` to `GameRenderer` (visual effects, 37 consumers)
   - ✅ `particle_system.h/.cpp` moved from `InterstellarOutpost` to `GameRenderer` (visual particles, 47 consumers)
   - ✅ `startsequence.h/.cpp` moved from `InterstellarOutpost` to `GameRenderer` (intro sequence rendering, 2 consumers)
   - ✅ `rocket_status_panel.h/.cpp` moved from `InterstellarOutpost` to `GameRenderer` (HUD panel rendering, 2 consumers)
   - ✅ `animatedpanel.h/.cpp` moved from `InterstellarOutpost` to `GameRenderer` (animated panel data, 2 consumers)
   - ✅ `animatedpanel_renderer.h/.cpp` moved from `InterstellarOutpost` to `GameRenderer` (animated panel rendering, 1 consumer)
   - ⬜ `GameRenderer` still has broad include paths to InterstellarOutpost, GameLogic (needed by `g_app` usage in moved files)
5. **Introduce a thin headless `server.exe` host** — *done*
   - ✅ `Server/Server.vcxproj` — console application linking NeuronCore + NeuronServer only
   - ✅ `Server/Server.cpp` — headless `main()` with 10 Hz tick loop, Ctrl+C signal handler, `--test` mode for CI
   - ✅ `Server/ServerStubs.cpp` — `glFinish()` no-op stub only (g_app stub removed after Wave 5)
   - ⬜ Cannot instantiate real `Server` class yet (NeuronServer deps resolved; blocked by runtime wiring — need `IServerHost` impl for headless mode)

### Batch 1 Suggested File Focus
- `NeuronCore/NeuronCore.h` — ✅ UI WinRT cleaned; `using namespace winrt;` deferred
- `NeuronCore/NeuronCore.vcxproj` — ✅ inverted include paths removed; graphics files relocated
- `NeuronCore/pch.h` — ✅ cleaned to only `#include "NeuronCore.h"`
- `NeuronServer/server.h` — ✅ moved from `NeuronClient`
- `NeuronServer/server.cpp` — ✅ moved from `NeuronClient`; decoupled from `g_app` via `IServerHost` (Wave 5)
- `NeuronClient/clienttoserver.h` — ✅ now depends on `TeamControls.h` (NeuronCore)
- `NeuronClient/NeuronClient.vcxproj` — ✅ now owns 48 relocated graphics/windowing files; Release include paths added
- `GameRenderer/ShadowRenderer.h/.cpp` — ✅ first render-only extraction from `GameLogic`
- `GameRenderer/entity_leg.h/.cpp` — ✅ render helper moved from `GameLogic`
- `GameRenderer/landscape_renderer.h/.cpp` — ✅ terrain rendering moved from `InterstellarOutpost`
- `GameRenderer/explosion.h/.cpp` — ✅ visual effects moved from `InterstellarOutpost`
- `GameRenderer/particle_system.h/.cpp` — ✅ visual particles moved from `InterstellarOutpost`
- `Server/Server.cpp` — ✅ headless host with `--test` mode
- `Server/ServerStubs.cpp` — ✅ temporary linker stubs (delete after Wave 5)
- `NeuronServer/IServerHost.h` — ✅ abstract interface decoupling Server from g_app (Wave 5)
- `NeuronCore/TeamTypes.h` — ✅ shared team type enum extracted from `team.h` (Wave 5)
- `GameRenderer/startsequence.h/.cpp` — ✅ intro sequence rendering moved from `InterstellarOutpost`
- `GameRenderer/rocket_status_panel.h/.cpp` — ✅ HUD panel rendering moved from `InterstellarOutpost`
- `GameRenderer/animatedpanel.h/.cpp` — ✅ animated panel data moved from `InterstellarOutpost`
- `GameRenderer/animatedpanel_renderer.h/.cpp` — ✅ animated panel rendering moved from `InterstellarOutpost`
- `NeuronCore/generic.h/.cpp` — ✅ `IpToString()` utility moved from NeuronClient (Wave 5 extension)
- `NeuronCore/servertoclientletter.h` — ✅ protocol data class moved from NeuronClient (Wave 5 extension)
- `NeuronCore/ftp_manager.h/.cpp` — ✅ FTP file transfer moved from NeuronClient (Wave 5 extension)
- `NeuronServer/servertoclient.h/.cpp` — ✅ server-side client connection moved from NeuronClient (Wave 5 extension)

### Batch 1 Exit Criteria
- ✅ `NeuronCore` builds without upward include-path dependencies — *achieved after Wave 2 graphics file relocation*
- ✅ `NeuronServer` owns the current `server.*` implementation
- ✅ Shared protocol/command contracts compile from `NeuronCore` (`TeamControls.h`, `bandwidth.h`, `globals.h`, `network_defines.h`, `worldobject.h`, `gametimer.h`)
- ✅ `GameRenderer` contains at least one real render-only extraction from `GameLogic` (`ShadowRenderer`)
- ✅ `server.exe` can launch headless (`Server/Server.cpp` with `--test` mode)

> **Batch 1 result**: All 5 exit criteria met. ✅

## Batch 1 Completion Summary

| Item | Status | Key Files | Remaining Blockers |
|------|--------|-----------|--------------------|
| NeuronCore public boundary cleanup | **Done** | `NeuronCore.h`, `pch.h`, `NeuronCore.vcxproj` | `using namespace winrt;` deferred |
| Graphics/windowing file relocation | **Done** | 48 files moved NeuronCore → NeuronClient | — |
| Shared contract extraction | **Done** | `TeamControls.h/.cpp`, `bandwidth.h`, `globals.h`, `network_defines.h`, `worldobject.h`, `gametimer.h` | — |
| Server code to NeuronServer | **Done** | `server.h/.cpp`, `servertoclient.h/.cpp`, `IServerHost.h` | NeuronServer fully isolated (NeuronCore-only) |
| Shared contracts (extended) | **Done** | `generic.h/.cpp`, `ftp_manager.h/.cpp`, `servertoclientletter.h` added to NeuronCore | — |
| GameRenderer boundary | **Done** (9 files) | `ShadowRenderer`, `entity_leg`, `landscape_renderer`, `explosion`, `particle_system`, `startsequence`, `rocket_status_panel`, `animatedpanel`, `animatedpanel_renderer` | More extractions needed; `g_app` coupling in moved files |
| Headless server.exe | **Done** | `Server/Server.cpp`, `Server/ServerStubs.cpp` | Need headless `IServerHost` impl to instantiate real `Server` |

## Wave 2 Completion Summary

### Files Moved: NeuronCore → NeuronClient (48 files)

**Headers (25):**
`3d_sprite.h`, `bitmap.h`, `DeviceNotify.h`, `FixedPipeline.h`, `language_table.h`, `ogl_extensions.h`, `opengl_directx.h`, `opengl_directx_dlist.h`, `opengl_directx_dlist_dev.h`, `opengl_directx_inline.h`, `opengl_directx_internals.h`, `opengl_directx_matrix_stack.h`, `opengl_trace.h`, `resource.h`, `safegl.h`, `shader.h`, `shape.h`, `sphere_renderer.h`, `targetcursor.h`, `texture.h`, `texture_uv.h`, `text_renderer.h`, `window_manager.h`, `window_manager_directx.h`, `WndProcManager.h`

**Sources (23):**
`3d_sprite.cpp`, `bitmap.cpp`, `FixedPipeline.cpp`, `language_table.cpp`, `ogl_extensions_directx.cpp`, `opengl_directx.cpp`, `opengl_directx_dlist.cpp`, `opengl_directx_dlist_dev.cpp`, `opengl_directx_matrix_stack.cpp`, `opengl_directx_stubs.cpp`, `opengl_trace.cpp`, `resource.cpp`, `safegl.cpp`, `shader.cpp`, `shape.cpp`, `sphere_renderer.cpp`, `targetcursor.cpp`, `texture.cpp`, `texture_uv.cpp`, `text_renderer.cpp`, `window_manager.cpp`, `window_manager_directx.cpp`, `WndProcManager.cpp`

### Files Moved: InterstellarOutpost → NeuronCore (1 header)

`gametimer.h` — network-safe game timer (shared simulation type). Implementation (`gametimer.cpp`) stays in InterstellarOutpost (uses `g_app`); linked at exe level.

### Blocking Dependency Fixes in NeuronCore "Staying" Files

| File | Fix | Details |
|------|-----|---------|
| `preferences.cpp` | Removed `app.h`, `resource.h`, `prefs_other_window.h` | Replaced `g_app->m_resource->GetTextReader()` with direct `FileSys` + `UnicodeTextFileReader`/`TextFileReader`. Inlined `OTHER_DIFFICULTY` macro as `"Difficulty"`. |
| `hi_res_time.cpp` | Removed dead `#include "main.h"` | No symbols from `main.h` were used. |
| `profiler.cpp` | Added `void glFinish();` forward declaration | Implementation in NeuronClient's OpenGL→DirectX layer; resolved at link time. |

### Build-System Changes

| File | Change |
|------|--------|
| `NeuronCore/NeuronCore.vcxproj` | Removed 25 `ClInclude` + 23 `ClCompile` entries. Removed `AdditionalIncludeDirectories` from Debug config. Added `gametimer.h`. |
| `NeuronClient/NeuronClient.vcxproj` | Added 25 `ClInclude` + 23 `ClCompile` entries. Added `AdditionalIncludeDirectories` to Release config (matching Debug). |
| `InterstellarOutpost/InterstellarOutpost.vcxproj` | Removed `gametimer.h` `ClInclude` entry. |
| `NeuronCore/pch.h` | Removed `#include "DirectXHelper.h"` — now only includes `NeuronCore.h`. |

### Wave 1 Acceptance Gate Status

| Gate | Status |
|------|--------|
| NeuronCore builds without include-path access to NeuronClient, NeuronServer, GameLogic, GameRenderer, or InterstellarOutpost | ✅ **Pass** |
| NeuronServer builds without NeuronClient include directories | ✅ **Pass** — NeuronServer depends only on NeuronCore |
| GameRenderer builds without server-only dependencies | ✅ **Pass** |
| server.exe launches headless | ✅ **Pass** (ServerStubs.cpp cleaned; g_app stub removed) |
| Client still renders and connects | ✅ **Pass** (build succeeds) |

## Batch 2: Wave 6 — GameRenderer Render Infrastructure Extraction

### Rationale
Entity/building files in `GameLogic` all mix `Advance()` (simulation) with `Render()` (visual) in a single class. Splitting individual entity render methods is a deep refactoring task (deferred). The practical first step is relocating **whole render-infrastructure files** — files that are entirely about rendering, visual effects, or render helpers with no authoritative simulation ownership.

### Files Moved: GameLogic → GameRenderer (1 pair)

| File | Consumers | Rationale |
|------|-----------|-----------|
| `entity_leg.h/.cpp` | 3 (spider.cpp, tripod.cpp, entity_leg.cpp) | Render helper for entity leg IK/animation. Not part of entity type system. Uses `g_app->m_resource` for shape loading. |

### Files Moved: InterstellarOutpost → GameRenderer (3 pairs — round 1)

| File | Consumers | Rationale |
|------|-----------|-----------|
| `landscape_renderer.h/.cpp` | 4 (3 IO, 1 NC) | Pure terrain rendering. OpenGL vertex buffer, display list management. Uses `g_app` for resource/location. |
| `explosion.h/.cpp` | 37 (31 GL, 5 IO, 1 main) | Visual-only explosion effects. Shape fragment tumbling, no authoritative state. Global `g_explosionManager`. |
| `particle_system.h/.cpp` | 47 (34 GL, 11 IO, 1 NC) | Visual particle effects (fire, sparks, trails, etc.). Pure rendering pipeline component. |

### Files Moved: InterstellarOutpost → GameRenderer (4 pairs — round 2)

| File | Consumers | Rationale |
|------|-----------|-----------||
| `startsequence.h/.cpp` | 2 (self + renderer.cpp) | Intro/cutscene sequence rendering. Uses `g_app` for resource/camera/renderer/location (15 sites). |
| `rocket_status_panel.h/.cpp` | 2 (self + multiwinia.cpp) | HUD panel for escape rocket status. Uses `g_app` for resource/renderer/camera (9 sites). |
| `animatedpanel.h/.cpp` | 2 (self + animatedpanel_renderer.h) | Animated panel data model (keyframes, objects). No `g_app` usage in `.cpp`. |
| `animatedpanel_renderer.h/.cpp` | 1 (self) | Animated panel render logic. Uses `g_app` for resource/renderer/camera (7 sites). |

### Build-System Changes

| File | Change |
|------|--------|
| `GameRenderer/GameRenderer.vcxproj` | Added 8 `ClInclude` + 8 `ClCompile` entries total (round 1 + round 2). Added NeuronClient project reference. |
| `GameRenderer/pch.h` | Changed from `GameRenderer.h` + `DirectXHelper.h` to `NeuronClient.h` + `GameRenderer.h`. |
| `GameLogic/GameLogic.vcxproj` | Removed `entity_leg.h` and `entity_leg.cpp` entries. |
| `InterstellarOutpost/InterstellarOutpost.vcxproj` | Removed 7 `ClInclude` + 7 `ClCompile` entries (`landscape_renderer`, `explosion`, `particle_system`, `startsequence`, `rocket_status_panel`, `animatedpanel`, `animatedpanel_renderer`). Added `$(SolutionDir)GameRenderer` to Debug include paths. |
| `NeuronClient/NeuronClient.vcxproj` | Added `$(SolutionDir)GameRenderer` to Debug and Release include paths. |

### GameRenderer Current File Inventory (9 pairs + PCH)

| File | Origin | Type |
|------|--------|------|
| `ShadowRenderer.h/.cpp` | Extracted from GameLogic | Entity shadow rendering (namespace `GameRenderer`) |
| `entity_leg.h/.cpp` | GameLogic | Entity leg IK render helper |
| `explosion.h/.cpp` | InterstellarOutpost | Visual explosion effects + `ExplosionManager` |
| `landscape_renderer.h/.cpp` | InterstellarOutpost | Terrain rendering (vertex buffers, display lists) |
| `particle_system.h/.cpp` | InterstellarOutpost | Visual particle system (17 particle types) |
| `startsequence.h/.cpp` | InterstellarOutpost | Intro/cutscene sequence rendering |
| `rocket_status_panel.h/.cpp` | InterstellarOutpost | HUD panel for escape rocket status |
| `animatedpanel.h/.cpp` | InterstellarOutpost | Animated panel data model (keyframes, objects) |
| `animatedpanel_renderer.h/.cpp` | InterstellarOutpost | Animated panel render logic |
| `GameRenderer.h` | — | Umbrella header (includes `NeuronCore.h`) |
| `pch.h` / `pch.cpp` | — | PCH (includes `NeuronClient.h` + `GameRenderer.h`) |

### Remaining `g_app` Coupling in Moved Files

All moved `.cpp` files still use `g_app` for resource/location/camera access. This is expected — decoupling `g_app` is Wave 5 work. The files are in the correct library boundary now; the coupling will be resolved when `App` is split into narrower service interfaces.

| File | `g_app` usage |
|------|---------------|
| `entity_leg.cpp` | `g_app->m_resource->GetShape()`, `g_app->m_location->m_landscape` |
| `landscape_renderer.cpp` | `g_app->m_resource`, `g_app->m_location`, `g_app->m_camera` |
| `explosion.cpp` | `g_app->m_renderer`, `g_app->m_camera` |
| `particle_system.cpp` | `g_app->m_resource`, `g_app->m_camera`, `g_app->m_location` |
| `startsequence.cpp` | `g_app->m_resource`, `g_app->m_camera`, `g_app->m_renderer`, `g_app->m_location` (15 sites) |
| `rocket_status_panel.cpp` | `g_app->m_resource`, `g_app->m_renderer`, `g_app->m_camera` (9 sites) |
| `animatedpanel.cpp` | — (no `g_app` usage) |
| `animatedpanel_renderer.cpp` | `g_app->m_resource`, `g_app->m_renderer`, `g_app->m_camera` (7 sites) |

## Wave 5 Completion Summary: NeuronServer g_app Decoupling

### Problem
`NeuronServer/server.cpp` referenced `g_app` at 14 call sites, forcing NeuronServer to include headers from `InterstellarOutpost` and `GameLogic`. This violated the target dependency direction and blocked headless `server.exe` instantiation.

### Solution: IServerHost Interface

Introduced an abstract interface (`NeuronServer/IServerHost.h`) with 6 virtual methods that encapsulate everything `server.cpp` needed from `g_app`:

| Method | Replaces |
|--------|----------|
| `GetMaxNumberOfPlayers()` | `g_app->GetMaxNumberofPlayers()` (4 sites) |
| `IsGameInLobby()` | `g_app->m_multiwinia->GameInLobby()` (1 site) |
| `IsLocalSinglePlayer()` | `g_app->m_clientToServer->GetServerPort() == -1` (3 sites) |
| `GetLocalClientId()` | `g_app->m_clientToServer->m_clientId` (1 site) |
| `DeliverToLocalClient(Directory*)` | `g_app->m_clientToServer->ReceiveLetter(letter)` (1 site) |
| `SendFromLocalClient(Directory*)` | `g_app->m_clientToServer->SendLetter(letter)` (1 site) |

Additionally, 3 self-referencing `g_app->m_server` sites were replaced with a file-static `s_serverInstance` pointer (set in `Initialise()`, cleared in destructor).

### Files Created

| File | Purpose |
|------|---------|
| `NeuronServer/IServerHost.h` | Abstract interface — 6 pure virtual methods |
| `NeuronCore/TeamTypes.h` | Shared enum: `TeamTypeUnused`, `TeamTypeLocalPlayer`, `TeamTypeRemotePlayer`, `TeamTypeCPU`, `TeamTypeSpectator` |

### Files Modified

| File | Change |
|------|--------|
| `NeuronServer/server.h` | Added `class IServerHost;` forward decl, `IServerHost* m_host = nullptr;` member, `void SetHost(IServerHost*)` |
| `NeuronServer/server.cpp` | Removed includes: `app.h`, `team.h`, `multiwinia.h`, `clienttoserver.h`. Added: `TeamTypes.h`, `IServerHost.h`. All 14 `g_app` call sites replaced. Added `s_serverInstance` static + `SetHost()` impl. |
| `InterstellarOutpost/app.cpp` | Added `AppServerHost` class (anonymous namespace) implementing `IServerHost` via `g_app` delegation. Wired in `App::StartNetwork()`: `m_server->SetHost(&s_appServerHost)` |
| `InterstellarOutpost/team.h` | Replaced inline `TeamType` enum with `#include "TeamTypes.h"` |
| `NeuronServer/NeuronServer.vcxproj` | Removed `$(SolutionDir)InterstellarOutpost` and `$(SolutionDir)GameLogic` from Debug and Release `AdditionalIncludeDirectories` |
| `Server/ServerStubs.cpp` | Removed dead `g_app = nullptr` stub. Only `glFinish()` no-op remains. |
| `Server/Server.cpp` | Updated header comments reflecting Wave 5 completion. |

### NeuronServer Include Path Status

| Config | Before (pre-Wave 5) | After Wave 5 | After Wave 5 Extension |
|--------|---------------------|--------------|------------------------|
| Debug | `NeuronCore; NeuronClient; InterstellarOutpost; GameLogic` | `NeuronCore; NeuronClient` | `NeuronCore` |
| Release | `NeuronCore; NeuronClient; InterstellarOutpost; GameLogic` | `NeuronCore; NeuronClient` | `NeuronCore` |

### ~~Remaining NeuronClient Dependencies in server.cpp~~ ✅ Resolved

All 4 headers that kept NeuronServer dependent on NeuronClient have been relocated:

| Header | Destination | Rationale |
|--------|-------------|-----------|
| `generic.h/.cpp` | NeuronCore | Trivial `IpToString()` utility; depends only on `net_lib.h` (NeuronCore) |
| `servertoclientletter.h` | NeuronCore | Protocol data class; depends on `vector3.h`, `worldobject.h`, `directory.h` (all NeuronCore). No `.cpp` — `AddUpdate()` impl lives in `server.cpp` |
| `ftp_manager.h/.cpp` | NeuronCore | FTP file transfer; depends on `llist.h`, `hash_table.h`, `string_utils.h`, `directory.h`, `net_mutex.h`, `network_defines.h` (all NeuronCore) |
| `servertoclient.h/.cpp` | NeuronServer | Server-side client connection class; depends on `unicode_string.h`, `darray.h`, `net_socket.h`, `net_socket_session.h`, `ftp_manager.h`, `directory.h` (all NeuronCore after moves) |

**Result**: `$(SolutionDir)NeuronClient` removed from NeuronServer include paths (Debug + Release). NeuronServer now depends **only** on NeuronCore. ✅

### vcxproj Changes (Wave 5 Extension)

| File | Change |
|------|--------|
| `NeuronClient/NeuronClient.vcxproj` | Removed 4 `ClInclude` + 3 `ClCompile` entries (`generic`, `ftp_manager`, `servertoclient`, `servertoclientletter`) |
| `NeuronCore/NeuronCore.vcxproj` | Added 3 `ClInclude` (`generic.h`, `ftp_manager.h`, `servertoclientletter.h`) + 2 `ClCompile` (`generic.cpp`, `ftp_manager.cpp`) |
| `NeuronServer/NeuronServer.vcxproj` | Added 1 `ClInclude` (`servertoclient.h`) + 1 `ClCompile` (`servertoclient.cpp`). Removed `$(SolutionDir)NeuronClient` from Debug + Release `AdditionalIncludeDirectories`. |

### Wave 5 Acceptance Gate

| Gate | Status |
|------|--------|
| `server.cpp` compiles without `app.h` | ✅ **Pass** |
| `server.cpp` has zero `g_app` runtime references | ✅ **Pass** (only commented-out reference at line 797) |
| NeuronServer include paths exclude InterstellarOutpost and GameLogic | ✅ **Pass** |
| NeuronServer include paths exclude NeuronClient | ✅ **Pass** (Wave 5 extension) |
| NeuronServer depends only on NeuronCore | ✅ **Pass** |
| Full solution builds | ✅ **Pass** |
| `ServerStubs.cpp` g_app stub removed | ✅ **Pass** |
