#include "color_utility.hlsli"

RWTexture2D<float4> gHdrTexture : register(u0);

StructuredBuffer<float> CurrentExposure : register(t0);

groupshared float4 gHdrColorCache[256];

[numthreads(256, 1, 1)]
void main(int3 _groupId : SV_GroupThreadID, int3 _dispatchId : SV_DispatchThreadID)
{
    gHdrColorCache[_groupId.x] = gHdrTexture[min(_dispatchId.xy, int2(1280, 720) - 1)];

    float4 color = gHdrColorCache[_groupId.x];
    color.rgb *= CurrentExposure[0];
    color.rgb = LinearToSRGB(ACESTonemapping(color.rgb));

    gHdrTexture[_dispatchId.xy] = color;
}
