#ifndef TRANS_SPEED
#define TRANS_SPEED (0.05f)
#endif

#ifndef EXPO_MIN
#define EXPO_MIN (0.01f)
#endif

#ifndef EXPO_MAX
#define EXPO_MAX (10.f)
#endif

#ifndef INV_FACTOR
#define INV_FACTOR (25.f)
#endif

RWStructuredBuffer<float> CurrentExposure : register(u0);

StructuredBuffer<float> PreExposure : register(t0);

groupshared float LuminCache[920];

[numthreads(920, 1, 1)]
void main(int3 _groupId : SV_GroupThreadID)
{
    int threadID = _groupId.y * 920 + _groupId.x;
    LuminCache[threadID] = CurrentExposure[threadID];

    GroupMemoryBarrierWithGroupSync();

    int stride = 23;
    int index = threadID / stride;
    float value = 0.f;

    if (threadID % stride == 0)
    {
        [unroll]
        for (int i = 0; i < 23; ++i)
        {
            value += LuminCache[index * 23 + i];
        }
    }

    GroupMemoryBarrierWithGroupSync();

    if (threadID % stride == 0)
    {
        LuminCache[index] = value / 23.f;
    }

    GroupMemoryBarrierWithGroupSync();

    stride = 8;
    index = threadID / stride;
    value = 0.f;

    if (threadID % stride == 0)
    {
        [unroll]
        for (int i = 0; i < 8; ++i)
        {
            value += LuminCache[index * 8 + i];
        }
    }

    GroupMemoryBarrierWithGroupSync();

    if (threadID % stride == 0)
    {
        LuminCache[index] = value / 8.f;
    }

    GroupMemoryBarrierWithGroupSync();

    value = 0.f;

    if (threadID == 0)
    {
        [unroll]
        for (int i = 0; i < 5; ++i)
        {
            value += LuminCache[i];
        }
        float averageLumin = value / 5.f + 0.001f;
        float preExpo = PreExposure[0];     // read pre-frame exposure
        float currExpo = lerp(preExpo, 1.f / INV_FACTOR / averageLumin, TRANS_SPEED);
        currExpo = clamp(currExpo, EXPO_MIN, EXPO_MAX);
        CurrentExposure[0] = currExpo;      // store final exposure at index 0
        CurrentExposure[1] = averageLumin;  // store average luminance at index 1
    }
}