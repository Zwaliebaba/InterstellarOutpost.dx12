---
name: add-sound
description: Add a new sound effect or ambient audio to the game. Use when wiring up a new 2D UI sound or a 3D positional sound for a game entity or world event.
argument-hint: "<SoundName> [2d|3d]"
context: fork
agent: Plan
---

# Add New Sound: $ARGUMENTS

Add a new sound effect to InterstellarOutpost.

## Steps

1. **Understand the audio system** — Read `NeuronClient/soundsystem.h` / `soundsystem.cpp` for the top-level audio manager. For 3D positional audio read `NeuronClient/sound_library_3d.h` and the XAudio2 backend `sound_library_3d_xaudio.h`. For 2D/UI sounds read `NeuronClient/sound_library_2d.h`. Check `NeuronClient/sound_instance.h` and `sound_parameter.h` for how individual sound instances and parameters are managed.

2. **Add the audio asset**:
   - Place the `.wav` / `.ogg` file in `InterstellarOutpost/Assets/sounds/` (or the existing asset sub-folder convention)
   - For streaming music, ensure the `sound_stream_decoder` can handle the format

3. **Register the sound**:
   - Add a string ID or enum constant for the new sound in `soundsystem.h` (or wherever existing sound names are declared)
   - Register the asset path in the sound system initialisation (`soundsystem.cpp`)

4. **Play the sound**:
   - For **3D positional** sounds: call the appropriate `sound_library_3d` play function, passing a world-space `vector3` position. Hook this call from the entity or event that triggers it (e.g., in `InterstellarOutpost/` entity update or `GameRenderer/` effect code).
   - For **2D / UI** sounds: call the `sound_library_2d` play function from the UI event handler in `GameLogic/` or `NeuronClient/eclipse.cpp`.
   - Use `sound_parameter` to set volume, pitch, and loop flags as needed.

5. **Apply sound filtering if needed** — For reverb or environmental effects, consult `NeuronClient/sound_filter.h`.

6. **Test with sample cache** — If the sound will be played frequently, ensure it is pre-loaded into `NeuronClient/sample_cache.h` to avoid runtime stalls.

## Conventions
- Keep audio assets at appropriate sample rates (44100 Hz preferred)
- 3D sounds must be short enough to not overlap badly when many instances play simultaneously — set a max-instance limit in the sound registration
- UI sounds should be 2D (non-positional) so they are unaffected by listener position
