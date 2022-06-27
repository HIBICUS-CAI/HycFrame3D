#include "color_utility.hlsli"

Texture2D<float4> OriginTex : register(t0);

RWStructuredBuffer<uint> Average : register(u1);

groupshared float SampleCache[256];

[numthreads(16, 16, 1)]
void main(int3 _groupId : SV_GroupThreadID, int3 _dispatchId : SV_DispatchThreadID)
{
    int linearGroupIndex = _groupId.x * 16 + _groupId.y;
    float3 originValue = OriginTex.Load(_dispatchId).rgb;
    SampleCache[linearGroupIndex] = RGBToLuminance(originValue);

    GroupMemoryBarrierWithGroupSync();

    float lineAverage = 0.f;
    if (_groupId.y == 0)
    {
        int index = 0;
        [unroll]
        for (int i = 0; i < 16; ++i)
        {
            index = 16 * i + _groupId.x;
            lineAverage += SampleCache[index];
        }
        lineAverage /= 16.f;
    }

    GroupMemoryBarrierWithGroupSync();

    if (_groupId.y == 0)
    {
        SampleCache[_groupId.x] = lineAverage;
    }

    GroupMemoryBarrierWithGroupSync();

    float groupAverage = 0.f;
    if (_groupId.x == 0 && _groupId.y == 0)
    {
        [unroll]
        for (int i = 0; i < 16; ++i)
        {
            groupAverage += SampleCache[i];
        }
        groupAverage /= 16.f;
        int origin = 0;
        InterlockedAdd(Average[0], asuint(groupAverage), origin);
    }
}
