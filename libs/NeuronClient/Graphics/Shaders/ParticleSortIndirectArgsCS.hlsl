
#include "ParticleUtility.hlsli"

RWByteAddressBuffer g_DispatchIndirectArgs : register(u0);
RWByteAddressBuffer g_DrawIndirectArgs : register(u1);

[RootSignature(Particle_RootSig)]
[numthreads(1, 1, 1)]
void main( uint GI : SV_GroupIndex )
{
    uint InstanceCount = g_DrawIndirectArgs.Load(4);
    uint ThreadGroupCount = (InstanceCount + 2047) / 2048;

    g_DispatchIndirectArgs.Store3(0, uint3(ThreadGroupCount, 1, 1));

    // Reset instance count so we can cull and determine how many we need to actually draw
    g_DrawIndirectArgs.Store(4, 0);
}
