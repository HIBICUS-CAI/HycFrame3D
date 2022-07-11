#include "color_utility.hlsli"

Texture2D<float4> OriginTex : register(t0);

RWStructuredBuffer<float> Average : register(u0);

groupshared float SampleCache[1024];

[numthreads(32, 32, 1)]
void main(int3 _groupId : SV_GroupThreadID, int3 _dispatchId : SV_DispatchThreadID, int3 _threadGroupID : SV_GroupID)
{
    int linearGroupIndex = _groupId.y * 32 + _groupId.x;
    float3 originValue = OriginTex.Load(_dispatchId).rgb;
    SampleCache[linearGroupIndex] = RGBToLuminance(originValue);

    GroupMemoryBarrierWithGroupSync();

    float lineAverage = 0.f;
    if (_groupId.y == 0)
    {
        int index = 0;
        [unroll]
        for (int i = 0; i < 32; ++i)
        {
            index = 32 * i + _groupId.x;
            lineAverage += SampleCache[index];
        }
        lineAverage /= 32.f;
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
        for (int i = 0; i < 32; ++i)
        {
            groupAverage += SampleCache[i];
        }
        groupAverage /= 32.f;
        Average[40 * _threadGroupID.y + _threadGroupID.x] = groupAverage;
    }
}
