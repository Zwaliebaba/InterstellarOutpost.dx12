---
name: add-shader
description: Add a new HLSL shader and wire it into the DirectX 12 pipeline. Use when creating a new rendering effect, post-process pass, or compute shader.
argument-hint: "<ShaderName> [vertex|pixel|compute|geometry]"
context: fork
agent: Plan
---

# Add New Shader: $ARGUMENTS

Add a new HLSL shader to the InterstellarOutpost DirectX 12 rendering pipeline.

## Steps

1. **Understand the existing pipeline** — Read `NeuronClient/shader.h` and `NeuronClient/shader.cpp` to understand how shaders are compiled and bound. Read `NeuronClient/GraphicsCore.h`/`GraphicsCore.cpp` for the DX12 device and command list setup. Review `NeuronClient/FixedPipeline.h` for the existing PSO patterns.

2. **Write the HLSL shader** — Create `InterstellarOutpost/Assets/<ShaderName>.hlsl` (or split into `<ShaderName>VS.hlsl` / `<ShaderName>PS.hlsl` if separate stages are needed):
   - Define root signature constants/CBV/SRV/UAV bindings via `[RootSignature(...)]` attribute or a separate RS file
   - Follow existing HLSL style — check `InterstellarOutpost/Assets/` for existing shader examples
   - Keep resource binding slots consistent with existing descriptors in `NeuronClient/GraphicsCore`

3. **Create/update the root signature** — If new bindings are required, update the relevant root signature in `NeuronClient/GraphicsCore.cpp`.

4. **Create the PSO** — In `NeuronClient/GraphicsCore.cpp` or the appropriate renderer:
   - Describe `D3D12_GRAPHICS_PIPELINE_STATE_DESC` (or `D3D12_COMPUTE_PIPELINE_STATE_DESC`)
   - Set input layout, blend state, rasterizer state, depth-stencil state to match the effect intent
   - Compile shaders at build time using the project's existing HLSL compilation setup

5. **Add draw/dispatch call** — In the appropriate renderer (`GameRenderer/` or `NeuronClient/`):
   - Set the PSO and root signature on the command list
   - Bind descriptor tables or root constants
   - Issue `DrawInstanced` / `DrawIndexedInstanced` / `Dispatch`
   - Insert required resource barriers (`D3D12_RESOURCE_BARRIER`) before and after

6. **Add to project** — Add the `.hlsl` file to `InterstellarOutpost/InterstellarOutpost.vcxproj` with the `FXCompile` item type.

## DX12 Correctness Checklist
- All resources must be in the correct state before use; insert `ResourceBarrier` transitions.
- Descriptor heaps: only one CBV/SRV/UAV heap and one sampler heap can be bound at a time.
- Root signature must match the shader's declared bindings exactly.
- GPU synchronisation: ensure fences are signalled/waited appropriately if the new pass reads resources written in a prior frame.
- Never call `Release()` on a resource that may still be in-flight on the GPU.
