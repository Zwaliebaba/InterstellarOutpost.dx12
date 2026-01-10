## DirectX 12 Sparse Voxel Ray Marching Prompt

You are an expert DirectX 12 graphics engineer tasked with building a real-time ray marched voxel scene powered by a Sparse Voxel Octree (SVO). Follow the requirements below to generate C++23 code and accompanying HLSL shaders that integrate cleanly with an existing Interstellar Outpost rendering framework.

### High-Level Objectives
- Initialize a DX12 rendering pipeline capable of dispatching compute shaders and presenting to the swap chain.
- Implement a GPU-friendly Sparse Voxel Octree asset that stores albedo, emissive, and occupancy data for each node.
- Perform ray marching in a compute shader that traverses the SVO, shades visible voxels, and writes the result into an unordered access texture for presentation.

### Detailed Requirements
1. **Sparse Voxel Tree Construction**
	- Provide CPU-side C++ utilities to ingest a dense voxel grid (e.g., a small biome chunk) and compress it into a pointer-less SVO stored in structured buffers.
		- Source voxel data is supplied via standard MagicaVoxel `.vox` files; include a loader that converts the .vox contents into the intermediate dense grid prior to compression.
	- Restrict the palette to 256 RGBA colors exactly as stored in the `.vox` file; leaf voxels store an 8-bit palette index plus occupancy, no additional material parameters.
	- Each node stores child bitmasks, palette indices, and bounding box scale information to enable efficient traversal without recursion.
	- Map `.vox` coordinates into world space assuming Y-up, 1 voxel = 0.25 meters, and chunk bounds of 128³ voxels centered at the origin.
	- Upload the SVO buffers to the GPU using default heap resources with upload heap staging.

2. **Ray Marching Compute Shader**
	- Write an HLSL compute shader that casts one ray per pixel from a dedicated voxel camera class.
	- Traverse the SVO using iterative stackless traversal; stop when a solid voxel is hit or the ray exits the world bounds.
	- Shade hits using palette colors modulated by a single directional light read from a DX12 constant buffer (`struct VoxelLightingCB { float3 lightDirWS; float intensity; float3 lightColor; float pad0; };`).
	- Apply emissive contribution straight from the palette’s alpha channel and exponential fog to misses.
	- Output final colors into a UAV-backed RGBA16F render target.

3. **Render Graph Integration**
	- Add a render pass that runs after camera matrices are updated but before post-processing.
	- Manage resource barriers for the ray marching output texture, then composite it into the main back buffer (full-screen triangle or blit).
	- Ensure descriptor heaps (SRV/UAV) are populated with the SVO buffers and result texture each frame.

4. **Camera & Controls**
	- Implement a bespoke `VoxelRayMarchCamera` class (not tied to existing camera systems) with configurable HFOV 80°, near plane 0.05 m, far plane 512 m.
	- Provide WASD + mouse look controls that move the camera through the voxel scene and allow runtime adjustment of movement speed.
	- Allow toggling between wireframe bounding boxes and fully shaded voxels for debugging.

5. **Diagnostics**
	- Emit GPU timing queries around the compute dispatch.
	- Add a Direct2D overlay rendered via DirectX11on12 interop that prints voxel count, tree depth, dispatch time, and current palette entry under the cursor.

### Implementation Notes
- Use C++23, winrt::check_hresult for HRESULT checking, winrt::com_ptr for DX12 objects, and RAII wrappers for command lists and fences.
- Organize code into Renderer modules consistent with the InterstellarOutpost.dx12 project (no monolithic functions).
- Provide clear TODO comments where engine-specific hooks must be connected.
- Assume the project already has a functioning DX12 device, swap chain, and command queue setup; focus on the SVO pipeline and ray marching pass.
- Budget for 1920×1080 output resolution at 60 FPS by capping SVO depth to 7 (root + 6 levels ≈ 128³ effective voxels) and using 8×8 thread groups (240×135 dispatch grid) with occupancy-based early exit.
- Define a dedicated root signature space for this feature: slot 0 = camera/lighting constant buffer, slot 1 = palette SRV, slot 2 = SVO node buffer SRV, slot 3 = ray march output UAV.

### Build & Shader Compilation
- Add a dedicated `VoxelRayMarching` static library (or module) to `src/CMakeLists.txt`, link it into `InterstellarOutpost` via the existing alias pattern, and expose include paths plus shader assets through that target.
- Author an `add_custom_command` that runs `dxc` at configure/build time to compile `VoxelRayMarchCS.hlsl` (and any supporting shaders) into DXIL blobs under `bin/<Config>/Shaders`, producing both stripped release DXIL and debug PDBs. Hook the custom command into the new target with `add_custom_target(VoxelRayMarchingShaders ... DEPENDS VoxelRayMarchCS.dxil)` so CMake rebuilds shaders when the source or included headers change.
- During engine startup (development builds), implement a lightweight runtime compilation path: check timestamps on the HLSL files, re-run `dxc` via `DxcCreateInstance2`, and hot-reload the resulting DXIL to support quick iteration. Guard this behind a config flag so shipping builds rely solely on the precompiled artifacts.
- Ensure the runtime selects the precompiled shader binaries first and falls back to recompilation only when the DXIL files are missing or stale, logging status via `DebugTrace`.

Deliverables should include C++ source snippets, HLSL shader code, and any required build system additions so the feature can be dropped into the game with minimal wiring.
