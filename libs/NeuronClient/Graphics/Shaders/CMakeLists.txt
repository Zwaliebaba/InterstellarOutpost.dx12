# Builds every .hlsl shader in this directory into a C header via DXC so the
# client can embed shader blobs directly in binaries.
cmake_minimum_required(VERSION 3.20)

project(ShaderHeaders LANGUAGES CXX)

# The DXC path comes from the parent build; fail early if it is not provided.
if(NOT DXC_EXECUTABLE)
    message(FATAL_ERROR "DXC_EXECUTABLE is not set—point it at your dxc.exe before configuring.")
endif()

# Stage-specific profiles are exposed as cache vars for quick overrides when debugging shaders.
set(COMPUTE_SHADER_TARGET_PROFILE "cs_6_7" CACHE STRING "DXC /T profile for compute shaders.")
set(VERTEX_SHADER_TARGET_PROFILE "vs_6_7" CACHE STRING "DXC /T profile for vertex shaders.")
set(PIXEL_SHADER_TARGET_PROFILE "ps_6_7" CACHE STRING "DXC /T profile for pixel shaders.")
set(SHADER_ENTRY_POINT "main" CACHE STRING "Default shader entry point passed to DXC (/E).")
set(DXC_DEBUG_ARGUMENT "$<$<CONFIG:Debug>:/Qembed_debug>" CACHE STRING "Automatically append /Qembed_debug when building Debug configs.")

# Emit headers next to the sources to simplify include paths for the client.
set(output_dir "${CMAKE_CURRENT_SOURCE_DIR}/Compiled")
file(MAKE_DIRECTORY "${output_dir}")

# Shader lists stay curated so we control exactly what hits the build.
set(COMPUTE_SHADER_FILES
    AdaptExposureCS.hlsl
    AoBlurUpsampleBlendOutCS.hlsl
    AoBlurUpsampleCS.hlsl
    AoBlurUpsamplePreMinBlendOutCS.hlsl
    AoBlurUpsamplePreMinCS.hlsl
    AoPrepareDepthBuffers1CS.hlsl
    AoPrepareDepthBuffers2CS.hlsl
    AoRender1CS.hlsl
    AoRender2CS.hlsl
    ApplyBloom2CS.hlsl
    ApplyBloomCS.hlsl
    AverageLumaCS.hlsl
    BicubicUpsampleCS.hlsl
    BicubicUpsampleFast16CS.hlsl
    BicubicUpsampleFast24CS.hlsl
    BicubicUpsampleFast32CS.hlsl
    Bitonic32InnerSortCS.hlsl
    Bitonic32OuterSortCS.hlsl
    Bitonic32PreSortCS.hlsl
    Bitonic64InnerSortCS.hlsl
    Bitonic64OuterSortCS.hlsl
    Bitonic64PreSortCS.hlsl
    BitonicIndirectArgsCS.hlsl
    BloomExtractAndDownsampleHdrCS.hlsl
    BloomExtractAndDownsampleLdrCS.hlsl
    BlurCS.hlsl
    BoundNeighborhoodCS.hlsl
    CameraMotionBlurPrePassCS.hlsl
    CameraMotionBlurPrePassLinearZCS.hlsl
    CameraVelocityCS.hlsl
    CopyBackPostBufferCS.hlsl
    DebugDrawHistogramCS.hlsl
    DebugLuminanceHdr2CS.hlsl
    DebugLuminanceHdrCS.hlsl
    DebugLuminanceLdr2CS.hlsl
    DebugLuminanceLdrCS.hlsl
    DebugSSAOCS.hlsl
    DoFCombine2CS.hlsl
    DoFCombineCS.hlsl
    DoFCombineFast2CS.hlsl
    DoFCombineFastCS.hlsl
    DoFDebugBlueCS.hlsl
    DoFDebugGreenCS.hlsl
    DoFDebugRedCS.hlsl
    DoFMedianFilterCS.hlsl
    DoFMedianFilterFixupCS.hlsl
    DoFMedianFilterSepAlphaCS.hlsl
    DoFPass1CS.hlsl
    DoFPass2CS.hlsl
    DoFPass2DebugCS.hlsl
    DoFPass2FastCS.hlsl
    DoFPass2FixupCS.hlsl
    DoFPreFilterCS.hlsl
    DoFPreFilterFastCS.hlsl
    DoFPreFilterFixupCS.hlsl
    DoFTilePassCS.hlsl
    DoFTilePassFixupCS.hlsl
    DownsampleBloomAllCS.hlsl
    DownsampleBloomCS.hlsl
    ExtractLumaCS.hlsl
    FXAAPass1_Luma2_CS.hlsl
    FXAAPass1_Luma_CS.hlsl
    FXAAPass1_RGB2_CS.hlsl
    FXAAPass1_RGB_CS.hlsl
    FXAAPass2H2CS.hlsl
    FXAAPass2HCS.hlsl
    FXAAPass2HDebug2CS.hlsl
    FXAAPass2HDebugCS.hlsl
    FXAAPass2V2CS.hlsl
    FXAAPass2VCS.hlsl
    FXAAPass2VDebug2CS.hlsl
    FXAAPass2VDebugCS.hlsl
    FXAAResolveWorkQueueCS.hlsl
    GenerateHistogramCS.hlsl
    GenerateMipsGammaCS.hlsl
    GenerateMipsGammaOddCS.hlsl
    GenerateMipsGammaOddXCS.hlsl
    GenerateMipsGammaOddYCS.hlsl
    GenerateMipsLinearCS.hlsl
    GenerateMipsLinearOddCS.hlsl
    GenerateMipsLinearOddXCS.hlsl
    GenerateMipsLinearOddYCS.hlsl
    LanczosCS.hlsl
    LanczosFast16CS.hlsl
    LanczosFast24CS.hlsl
    LanczosFast32CS.hlsl
    LinearizeDepthCS.hlsl
    MotionBlurFinalPassCS.hlsl
    MotionBlurPrePassCS.hlsl
    ParticleBinCullingCS.hlsl
    ParticleDepthBoundsCS.hlsl
    ParticleDispatchIndirectArgsCS.hlsl
    ParticleFinalDispatchIndirectArgsCS.hlsl
    ParticleLargeBinCullingCS.hlsl
    ParticlePreSortCS.hlsl
    ParticleSortIndirectArgsCS.hlsl
    ParticleSpawnCS.hlsl
    ParticleTileCullingCS.hlsl
    ParticleTileRender2CS.hlsl
    ParticleTileRenderCS.hlsl
    ParticleTileRenderFast2CS.hlsl
    ParticleTileRenderFastCS.hlsl
    ParticleTileRenderFastDynamic2CS.hlsl
    ParticleTileRenderFastDynamicCS.hlsl
    ParticleTileRenderFastLowRes2CS.hlsl
    ParticleTileRenderFastLowResCS.hlsl
    ParticleTileRenderSlowDynamic2CS.hlsl
    ParticleTileRenderSlowDynamicCS.hlsl
    ParticleTileRenderSlowLowRes2CS.hlsl
    ParticleTileRenderSlowLowResCS.hlsl
    ParticleUpdateCS.hlsl
    ResolveTAACS.hlsl
    SharpenTAACS.hlsl
    TemporalBlendCS.hlsl
    ToneMap2CS.hlsl
    ToneMapCS.hlsl
    ToneMapHDR2CS.hlsl
    ToneMapHDRCS.hlsl
    UpsampleAndBlurCS.hlsl
)

set(VERTEX_SHADER_FILES
    ParticleNoSortVS.hlsl
    ParticleVS.hlsl
    PerfGraphBackgroundVS.hlsl
    PerfGraphVS.hlsl
    ScreenQuadCommonVS.hlsl
    ScreenQuadPresentVS.hlsl
    TextVS.hlsl
)

set(PIXEL_SHADER_FILES
    BicubicHorizontalUpsamplePS.hlsl
    BicubicUpsampleGammaPS.hlsl
    BicubicUpsamplePS.hlsl
    BicubicVerticalUpsamplePS.hlsl
    BilinearUpsamplePS.hlsl
    BlendUIHDRPS.hlsl
    BufferCopyPS.hlsl
    CompositeHDRPS.hlsl
    CompositeSDRPS.hlsl
    DownsampleDepthPS.hlsl
    LanczosHorizontalPS.hlsl
    LanczosVerticalPS.hlsl
    MagnifyPixelsPS.hlsl
    MotionBlurFinalPassPS.hlsl
    ParticlePS.hlsl
    PerfGraphPS.hlsl
    PresentHDRPS.hlsl
    PresentSDRPS.hlsl
    ScaleAndCompositeHDRPS.hlsl
    ScaleAndCompositeSDRPS.hlsl
    SharpeningUpsampleGammaPS.hlsl
    SharpeningUpsamplePS.hlsl
    TextAntialiasPS.hlsl
    TextShadowPS.hlsl
)

# add_shader_headers(<list-var> <profile>) keeps the command logic in one place.
function(add_shader_headers shader_list target_profile)
    foreach(shader_relative IN LISTS ${shader_list})
        set(shader_file "${CMAKE_CURRENT_SOURCE_DIR}/${shader_relative}")
        get_filename_component(shader_name "${shader_file}" NAME_WE)
        set(header_path "${output_dir}/${shader_name}.h")

        add_custom_command(
            OUTPUT "${header_path}"
            COMMAND "${DXC_EXECUTABLE}"
                /Zi          
                ${DXC_DEBUG_ARGUMENT}
                /nologo
                /T "${target_profile}"
                /E "${SHADER_ENTRY_POINT}"
                /Fh "${header_path}"
                /Vn "g_p${shader_name}"
                "${shader_file}"
            DEPENDS "${shader_file}"
            COMMENT "DXC ${shader_name}.hlsl -> ${header_path}"
            VERBATIM
        )

        list(APPEND generated_headers "${header_path}")
        set(generated_headers "${generated_headers}" PARENT_SCOPE)
    endforeach()
endfunction()

# Generate headers for each shader group and wire them into a single custom target.
set(generated_headers)
add_shader_headers(COMPUTE_SHADER_FILES "${COMPUTE_SHADER_TARGET_PROFILE}")
add_shader_headers(VERTEX_SHADER_FILES "${VERTEX_SHADER_TARGET_PROFILE}")
add_shader_headers(PIXEL_SHADER_FILES "${PIXEL_SHADER_TARGET_PROFILE}")

if(generated_headers)
    add_custom_target(ShaderHeaders ALL DEPENDS ${generated_headers})
else()
    add_custom_target(ShaderHeaders ALL
        COMMENT "No HLSL shaders with CS/VS/PS suffixes were found in ${CMAKE_CURRENT_SOURCE_DIR}.")
endif()
