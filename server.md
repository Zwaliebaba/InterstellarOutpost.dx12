# Client/Server Library Split Analysis Plan

## Goal
Create a repeatable way to analyze the current codebase and estimate the impact of moving `.cpp` and `.h` files into four target libraries:

- `NeuronCore` - shared by client and server
- `NeuronClient` - client-only orchestration, input, audio, UI, and platform integration
- `NeuronServer` - server-only authority, simulation hosting, and server transport/orchestration
- `GameRenderer` - client-side render and presentation logic extracted from `GameLogic`

This plan is for analysis and impact assessment first, not for bulk moves.

## Current-State Facts To Anchor The Analysis

- `InterstellarOutpost/main.cpp` is the current top-level loop and pulls in rendering, input, gameplay, client networking, and server headers in one place (`InterstellarOutpost/main.cpp:1-53`).
- `App` currently owns both client-facing and server-facing subsystems, including `Renderer`, `SoundSystem`, `Camera`, `LocationInput`, `TaskManagerInterface`, `GameMenu`, `MarkerSystem`, `Server`, and `ClientToServer` (`InterstellarOutpost/app.h:48-108`).
- `App` also exposes global access through `g_app` (`InterstellarOutpost/app.h:239-239`).
- `NeuronCore` currently includes WinRT and Windows UI headers directly in its umbrella header (`NeuronCore/NeuronCore.h:78-93`) and also includes `DirectXHelper.h` (`NeuronCore/NeuronCore.h:101-101`).
- `NeuronCore` currently compiles graphics/windowing files such as `window_manager_directx.cpp`, `opengl_directx.cpp`, `sphere_renderer.cpp`, and `text_renderer.cpp` (`NeuronCore/NeuronCore.vcxproj:194-229`).
- `NeuronCore` currently has **Debug-only** include paths into `NeuronClient`, `GameLogic`, and `InterstellarOutpost`, which means the dependency direction is already inverted (`NeuronCore/NeuronCore.vcxproj:56`). The Release configuration does not have these paths (`NeuronCore/NeuronCore.vcxproj:64-80`), so the Release build may already fail to resolve `DirectXHelper.h` (which lives in `NeuronClient/`, not `NeuronCore/`).
- `NeuronCore/NeuronCore.h:95` contains `using namespace winrt;`, which pollutes every consumer's namespace and violates the project rule against `using namespace` in headers.
- `NeuronClient` already carries Windows App SDK package imports (`NeuronClient/NeuronClient.vcxproj:3-15`).
- `NeuronClient` currently contains both client and server networking code, including `clienttoserver.*`, `server.*`, and `servertoclient.*` (`NeuronClient/NeuronClient.vcxproj:95-156`, `NeuronClient/NeuronClient.vcxproj:158-208`).
- `NeuronServer` exists as a static library project, but it is effectively empty today and only contains PCH files plus a dependency on `NeuronCore` (`NeuronServer/NeuronServer.vcxproj:14-27`).
- `NeuronServer` also currently has **Debug-only** include paths into `NeuronClient` and `InterstellarOutpost`, so it is not yet isolated (`NeuronServer/NeuronServer.vcxproj:73`). Release has no upward paths.
- `NeuronServer/NeuronServer.h` includes `NeuronCore.h`, so `NeuronServer` is **transitively poisoned** by all UI WinRT, DirectX, and `DirectXHelper.h` leakage from the NeuronCore umbrella. Step 1 (NeuronCore cleanup) must complete before NeuronServer can compile cleanly.
- `ClientToServer` currently depends on gameplay types such as `TeamControls` via `team.h`, so the client networking layer is not cleanly separated from gameplay (`NeuronClient/clienttoserver.h:4-10`, `NeuronClient/clienttoserver.h:159-192`).
- The `team.h` dependency has a deep transitive chain: `clienttoserver.h` → `team.h` (`InterstellarOutpost/`) → `entity.h` (`GameLogic/`) → `worldobject.h` (`GameLogic/`) + `shape.h` (`NeuronCore/`, graphics). Contract extraction must break this chain, not just extract payloads.
- `NeuronClient/server.h` includes `bandwidth.h` (`NeuronClient/bandwidth.h`), so `server.h` cannot move to `NeuronServer` until `bandwidth.h` moves to `NeuronCore`.
- `InterstellarOutpost/globals.h` defines fundamental shared constants (`NUM_TEAMS`, `GRAVITY`, `IAMALIVE_PERIOD`). It is included by `building.h` and `network_defines.h` (`NeuronClient/`), making it a shared contract currently trapped in the host project.
- `Server` is physically located in `NeuronClient`, which is the clearest immediate misplacement for the target architecture (`NeuronClient/server.h:41-143`).
- `g_app` is a major coupling hotspot: there are `6794` matches across `199` source files in `NeuronCore`, `NeuronClient`, `NeuronServer`, `GameLogic`, and `InterstellarOutpost`.

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

- `server.*` currently misplaced under `NeuronClient` (`NeuronClient/NeuronClient.vcxproj:137-138`, `NeuronClient/NeuronClient.vcxproj:191-192`)
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
1. Remove inverted include paths from `NeuronCore` debug settings (`NeuronCore/NeuronCore.vcxproj:47-57`).
2. Remove `NeuronClient` and `InterstellarOutpost` include leakage from `NeuronServer` (`NeuronServer/NeuronServer.vcxproj:64-74`).
3. Remove any upward include-path dependency from `NeuronCore` into `GameRenderer` or host code.
4. Make project references reflect intended direction before moving many files.
5. Keep PCHs minimal and library-local.

### Wave 2 - Public Header Detox
1. Remove `DirectXHelper.h` and other client-only graphics dependencies from `NeuronCore` umbrella and public headers.
2. Remove UI-related WinRT headers from `NeuronCore` umbrella and PCH files.
3. Remove `using namespace winrt;` from `NeuronCore/NeuronCore.h:95` (violates header namespace pollution rule).
4. Replace public heavy includes with forward declarations wherever possible.
5. Isolate render-only APIs away from shared or simulation-facing headers.

### Wave 3 - Extract Shared Contracts
Move or create shared headers in `NeuronCore` for:

- **prerequisite file moves**: `bandwidth.h` (from `NeuronClient/`), `globals.h` (from `InterstellarOutpost/`), `network_defines.h` (from `NeuronClient/`) — these are small, low-risk shared definitions that block later steps
- `worldobject.h` (from `GameLogic/`) — base class for `Entity` and `Building`, depends only on `vector3.h` (NeuronCore)
- packet/message enums
- transport-neutral DTOs
- deterministic gameplay commands
- team/input command payloads currently shared across client/server
- shared simulation state that must be understood by both client prediction/rendering and server authority

This is needed because `ClientToServer` already pulls gameplay state through `team.h` (`NeuronClient/clienttoserver.h:10-10`, `NeuronClient/clienttoserver.h:159-192`).

### Wave 4 - Move Obvious Server Code
Move the clearest server-only files first:

- `NeuronClient/server.h`
- `NeuronClient/server.cpp`
- any server-side session/history/authority files that do not require UI or renderer code

Do not move them until their includes no longer rely on client-only headers.

### Wave 5 - Split `App`
`App` is currently a mixed owner of client and server state (`InterstellarOutpost/app.h:48-108`).

Break it into narrower facades and service boundaries:

- `ClientAppContext`
- `ServerAppContext`
- `SharedGameContext` or equivalent shared services layer
- smaller service interfaces such as rendering, simulation, audio, and networking services where possible

Goal: reduce direct `g_app` access and replace it with explicit dependencies.

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
1. **Clean `NeuronCore` public boundaries**
   - remove `DirectXHelper.h` and other client-only graphics dependencies from `NeuronCore` public umbrella/PCH usage
   - remove UI-related WinRT headers from `NeuronCore`
   - stop `NeuronCore` from relying on upward include paths into client or host code
2. **Create or harden shared contracts in `NeuronCore`**
   - extract packet/message enums, shared command payloads, IDs, and simulation-facing DTOs
   - remove direct gameplay-heavy dependencies from client/server transport headers where possible
3. **Move obvious server-only code into `NeuronServer`**
   - move `server.h` / `server.cpp` out of `NeuronClient` only after header cleanup
   - keep server authority free of renderer, UI, and Windows App SDK dependencies
4. **Stand up the `GameRenderer` boundary**
   - identify the first render-only files in `GameLogic` that can move safely
   - move only rendering/presentation code, not authoritative update logic
5. **Introduce a thin headless `server.exe` host**
   - minimal startup, fixed tick loop, clean shutdown, no windowing or DirectX

### Batch 1 Suggested File Focus
- `NeuronCore/NeuronCore.h`
- `NeuronCore/NeuronCore.vcxproj`
- `NeuronClient/server.h`
- `NeuronClient/server.cpp`
- `NeuronClient/clienttoserver.h`
- first render-only files extracted from `GameLogic` into `GameRenderer`

### Batch 1 Exit Criteria
- `NeuronCore` builds without upward include-path dependencies
- `NeuronServer` owns the current `server.*` implementation
- shared protocol/command contracts compile from `NeuronCore`
- `GameRenderer` contains at least one real render-only extraction from `GameLogic`
- `server.exe` can launch headless

## Batch 1 File-by-File Checklist

Use this checklist to keep the first migration batch small, reviewable, and low-risk.

### `NeuronCore/NeuronCore.h`
- [ ] remove `DirectXHelper.h` from the public umbrella include set
- [ ] remove UI-related WinRT headers from the public umbrella include set
- [ ] keep only standard WinRT headers that are valid for both client and server
- [ ] verify that shared headers no longer rely on client-only transitive includes

### `NeuronCore/pch.h`
- [ ] keep the PCH limited to core-safe includes only
- [ ] verify that the PCH does not pull in DirectX, Windows App SDK, or UI-related WinRT indirectly

### `NeuronCore/NeuronCore.vcxproj`
- [ ] remove upward include-path dependencies into `NeuronClient`, `GameLogic`, `GameRenderer`, and `InterstellarOutpost`
- [ ] review graphics- and windowing-oriented compilation units and mark each one as `Stay`, `Move`, or `Split`
- [ ] ensure `NeuronCore` can build with only core-owned include paths and references

### `NeuronClient/server.h`
- [ ] move to `NeuronServer`
- [ ] remove any dependence on client-only headers or client-only transitive includes
- [ ] confirm that the public interface depends only on `NeuronCore` and server-safe types

### `NeuronClient/server.cpp`
- [ ] move to `NeuronServer`
- [ ] remove any client-only implementation dependencies
- [ ] verify that the implementation can compile without Windows App SDK, DirectX, or UI code

### `NeuronClient/clienttoserver.h`
- [ ] identify gameplay-heavy dependencies pulled through `team.h`
- [ ] extract shared command payloads and protocol-facing DTOs into `NeuronCore`
- [ ] reduce the public interface to shared contract types plus client transport responsibilities

### `NeuronClient/clienttoserver.cpp`
- [ ] update includes after shared payload extraction
- [ ] confirm that client transport stays in `NeuronClient`
- [ ] remove dependencies on gameplay implementation details where possible

### `NeuronServer/NeuronServer.vcxproj`
- [ ] add moved server source files
- [ ] remove `NeuronClient` and host include leakage
- [ ] verify references and include paths match the intended server-only dependency direction

### `InterstellarOutpost/main.cpp`
- [ ] keep in the host layer for Batch 1
- [ ] identify startup/shutdown logic that should remain host-only
- [ ] note any direct dependencies that block a future split into `client.exe` and `server.exe`

### `InterstellarOutpost/app.h`
- [ ] keep in the host layer for Batch 1
- [ ] identify members that are clearly client-only, server-only, or shared-service ownership
- [ ] record which fields and APIs are major blockers for reducing `g_app` coupling

### `GameLogic/entity.h` and `GameLogic/entity.cpp`
- [ ] mark as `Split First`, not `Move First`
- [ ] identify render-facing APIs, render data, and shape/visual dependencies
- [ ] define the first candidate extraction into `GameRenderer` without moving authoritative update logic

### `GameLogic/building.h` and `GameLogic/building.cpp`
- [ ] mark as `Split First`, not `Move First`
- [ ] identify render-facing APIs, render data, and shape/visual dependencies
- [ ] define the first candidate extraction into `GameRenderer` without moving authoritative update logic

### `GameRenderer/` first extraction candidates
- [ ] create or confirm the project boundary for `GameRenderer`
- [ ] move one real render-only path from `GameLogic` into `GameRenderer`
- [ ] keep extracted code dependent on shared simulation state, not server authority
- [ ] verify the extracted code does not become a second host/orchestration layer

### `server.exe` host
- [ ] create a thin headless startup path
- [ ] link only `NeuronCore` and `NeuronServer`
- [ ] ensure there is no window creation, DirectX initialization, or Windows App SDK dependency
- [ ] add basic fixed-tick startup and clean shutdown behavior suitable for container hosting

## Wave Acceptance Gates

Each wave should finish with explicit pass/fail checks:

1. `NeuronCore` builds without include-path access to `NeuronClient`, `NeuronServer`, `GameLogic`, `GameRenderer`, or `InterstellarOutpost`.
2. `NeuronServer` builds without `NeuronClient` include directories.
3. `GameRenderer` builds without server-only dependencies.
4. `server.exe` launches headless with no window creation, DirectX, or Windows App SDK dependency.
5. The client still renders and connects successfully.
6. Protocol compatibility tests between client and server still pass.
7. Deterministic server update tests still pass for authoritative game-object updates.

## Minimum Deliverables From The Analysis

1. A migration spreadsheet covering every `.cpp` and `.h`.
2. A dependency heat map showing cross-library includes.
3. A list of files that are safe to move now.
4. A list of files that must be split first.
5. A list of public headers that need contract extraction.
6. A list of files that should move from `GameLogic` to `GameRenderer`.
7. A build-system change list for each `.vcxproj`.
8. A short ADR for each major rule or exception.

## First Files To Review In Detail

Start with these because they define the current boundaries:

- `InterstellarOutpost/main.cpp` (`InterstellarOutpost/main.cpp:1-53`)
- `InterstellarOutpost/app.h` (`InterstellarOutpost/app.h:48-108`)
- `NeuronCore/NeuronCore.h` (`NeuronCore/NeuronCore.h:78-103`)
- `NeuronCore/NeuronCore.vcxproj` (`NeuronCore/NeuronCore.vcxproj:47-57`, `NeuronCore/NeuronCore.vcxproj:194-229`)
- `NeuronClient/NeuronClient.vcxproj` (`NeuronClient/NeuronClient.vcxproj:3-15`, `NeuronClient/NeuronClient.vcxproj:95-208`)
- `NeuronServer/NeuronServer.vcxproj` (`NeuronServer/NeuronServer.vcxproj:14-27`, `NeuronServer/NeuronServer.vcxproj:64-74`)
- `NeuronClient/server.h` (`NeuronClient/server.h:41-143`)
- `NeuronClient/clienttoserver.h` (`NeuronClient/clienttoserver.h:4-10`, `NeuronClient/clienttoserver.h:159-192`)
- `GameRenderer/` for the first render-only extraction candidates

## Suggested Output Format For Each Reviewed File

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
- Move readiness: Ready / Split First / Leave In Host
- Risk: Low / Medium / High
- Required follow-up:
```

## Resolved Decisions

These architecture decisions are now fixed for the migration plan:

1. The target architecture uses four library boundaries: `NeuronCore`, `NeuronClient`, `NeuronServer`, and `GameRenderer`.
2. `GameLogic` should be split by responsibility: server-side authoritative updates for game objects stay on the server path, while client render logic is migrated out into `GameRenderer`.
3. `GameRenderer` is a first-class client-side library boundary for render and presentation logic extracted from `GameLogic`.
4. A dedicated headless `server.exe` is required as a future deliverable for container hosting.
5. `NeuronCore` may keep standard WinRT support, but UI-related WinRT headers should be removed from it.
6. Shared protocol/message definitions should live in `NeuronCore`.

## Remaining Clarifications

These are still worth confirming before implementation:

1. Should client-side visual representations remain alongside the existing gameplay classes, or do you want explicit paired types such as simulation objects vs render proxies/components?
2. Do you want the first milestone to produce only clean static-library boundaries, or also introduce the new client and server executable hosts in the same change?
3. Should the future `server.exe` support dedicated offline simulation/testing modes in addition to networked authoritative hosting?
