---
name: review-dx12
description: Review DirectX 12 rendering code for correctness, performance, and best practices. Use when auditing GPU resource management, command lists, barriers, or descriptor heaps.
argument-hint: "[file or area to review]"
context: fork
agent: Explore
---

# DirectX 12 Code Review: $ARGUMENTS

Review the DirectX 12 code in `$ARGUMENTS` (or the whole `NeuronClient/` rendering layer if no argument given) for correctness, performance, and best practices.

## Review Checklist

### Resource Barriers
- [ ] All resources transition to the correct state before use (`D3D12_RESOURCE_STATE_*`)
- [ ] No missing barriers between render-target writes and subsequent reads as SRV
- [ ] Batch barriers into a single `ResourceBarrier()` call where possible
- [ ] UAV barriers present between write and subsequent read of a UAV resource

### Descriptor Heaps & Root Signature
- [ ] Only one CBV/SRV/UAV heap and one sampler heap bound per command list at a time
- [ ] Root signature declarations match shader register bindings exactly
- [ ] Descriptor handles are not invalidated by re-creating the heap mid-frame
- [ ] Static samplers used where samplers don't change per-draw

### Command Lists & Queues
- [ ] Command lists are reset before recording each frame
- [ ] `Close()` is called before `ExecuteCommandLists()`
- [ ] No raw pointers to command allocators re-used before the GPU has finished
- [ ] Bundles (if used) are recorded once and not modified while in-flight

### Synchronisation & Fences
- [ ] Fence signal/wait pairs correctly bracket GPU work before CPU reads back results
- [ ] No `WaitForSingleObject` on a fence that has not yet been signalled
- [ ] Frame resources (per-frame CBs, upload heaps) are double/triple-buffered correctly
- [ ] No `Release()` on a resource before GPU work referencing it has completed

### Memory & Upload Heaps
- [ ] Temporary upload buffers are kept alive until the copy command completes on the GPU
- [ ] Large static assets placed in `D3D12_HEAP_TYPE_DEFAULT`, not `UPLOAD`
- [ ] Committed vs placed resources chosen deliberately; no unnecessary committed allocations

### Pipeline State Objects
- [ ] PSO creation cached; not re-created every frame
- [ ] Depth-stencil state matches the actual depth buffer format
- [ ] Blend state and rasteriser state are intentional (e.g. not accidentally leaving back-face culling off)

### General Correctness (referencing project files)
- [ ] `NeuronClient/GraphicsCore.cpp` device/queue setup is sound
- [ ] `NeuronClient/shader.cpp` shader compilation errors are not silently swallowed
- [ ] `NeuronClient/TextureManager.cpp` textures released correctly on shutdown
- [ ] `NeuronClient/opengl_directx.cpp` OpenGL emulation layer is not bypassing DX12 correctness

## Output
Produce a structured report:
1. **Critical issues** — bugs that will cause crashes or corruption
2. **Correctness issues** — incorrect but not immediately crashing (e.g. missing barrier)
3. **Performance issues** — wasteful patterns (e.g. per-frame PSO creation, unnecessary readback)
4. **Minor / style** — naming, dead code, TODO items
