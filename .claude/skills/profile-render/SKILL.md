---
name: profile-render
description: Identify and fix rendering performance bottlenecks. Use when frame rate is low, GPU is over-budget, or specific rendering passes are slow.
argument-hint: "[area to investigate: e.g. particles, landscape, shadows]"
context: fork
agent: Explore
---

# Render Performance Profile: $ARGUMENTS

Investigate and resolve rendering performance bottlenecks in `$ARGUMENTS` (or the full pipeline if no area specified).

## Investigation Steps

### 1. Read the Rendering Pipeline
- `NeuronClient/GraphicsCore.cpp` — device, swap chain, per-frame command list submission
- `GameRenderer/landscape_renderer.cpp` — terrain draw calls
- `GameRenderer/particle_system.cpp` — particle rendering
- `GameRenderer/explosion.cpp` — explosion effects
- `GameRenderer/entity_leg.cpp` and `GameRenderer/animatedpanel_renderer.cpp` — entity rendering
- `NeuronClient/sphere_renderer.cpp`, `NeuronClient/text_renderer.cpp` — utility renderers
- `NeuronClient/shadow_renderer.cpp` (if present) or `GameRenderer/ShadowRenderer.cpp`

### 2. Common Bottleneck Patterns to Look For

**CPU-side (draw call overhead)**
- Excessive per-draw-call state changes (PSO, root signature, descriptor table)
- Missing instancing for repeated geometry (entities, particles)
- Dynamic vertex buffer uploads every frame that could be static
- Too many small constant buffer updates — batch into arrays

**GPU-side (shader cost)**
- Overdraw in particle systems — check blend state and depth test
- Landscape rendered at full resolution with no LOD
- Shadow map resolution too high or updated every frame unnecessarily
- Texture sampling with poor mip selection (missing `GenerateMips` or incorrect LOD bias)

**Synchronisation stalls**
- CPU waiting on GPU fence mid-frame (check `GraphicsCore` frame-sync logic)
- Upload heap writes that block because the buffer is still in use
- Readback operations (screenshots, occlusion queries) blocking the render loop

**Descriptor / heap inefficiency**
- Descriptor heaps rebuilt every frame — cache and update incrementally
- Root descriptor table with too many unused slots

### 3. Recommended Fixes
For each bottleneck found, propose a concrete fix with the specific file and line range, explaining:
- What the problem is
- What the fix is
- Expected performance impact

### 4. Output Format
Produce a prioritised list:
1. **High impact** — likely to give >10% frame time saving
2. **Medium impact** — worthwhile but secondary
3. **Low impact / future work** — good to track but not urgent
