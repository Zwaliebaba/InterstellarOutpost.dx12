
#include "PresentRS.hlsli"
#include "ColorSpaceUtility.hlsli"

Texture2D<float3> MainBuffer : register(t0);

[RootSignature(Present_RootSig)]
float3 main( float4 position : SV_Position, float2 uv : TexCoord0 ) : SV_Target0
{
    return ApplyREC2084Curve(MainBuffer[(int2)position.xy] / 10000.0);
}
