#ifndef EDGE_THRESHOLD
#define EDGE_THRESHOLD (0.125f)
#endif

#ifndef MIN_EDGE_THRESHOLD
#define MIN_EDGE_THRESHOLD (0.0625f)
#endif

#ifndef EDGE_SEARCH_STEP
#define EDGE_SEARCH_STEP (10)
#endif

#ifndef EDGE_GUESS
#define EDGE_GUESS (8)
#endif

#include "color_utility.hlsli"

RWTexture2D<float4> SourceTex : register(u0);
Texture2D<float4> CopyTex : register(t0);

SamplerState LinearBorder : register(s0);

groupshared float LuminCache[(16 + 2 * EDGE_SEARCH_STEP) * (16 + 2 * EDGE_SEARCH_STEP)];

int Index2DTo1D(int2 _index)
{
    return _index.y * 16 + _index.x;
}

int2 Index1DTo2D(int _index)
{
    return int2(_index % 16, _index / 16);
}

int Index2DTo1DEx(int2 _index)
{
    return _index.y * (16 + 2 * EDGE_SEARCH_STEP) + _index.x;
}

int2 Index1DTo2DEx(int _index)
{
    return int2(_index % (16 + 2 * EDGE_SEARCH_STEP), _index / (16 + 2 * EDGE_SEARCH_STEP));
}

[numthreads(16, 16, 1)]
void main(int3 _groupId : SV_GroupThreadID, int3 _dispatchId : SV_DispatchThreadID)
{
    if (_groupId.x == 0)
    {
        [unroll]
        for (int i = 0; i < EDGE_SEARCH_STEP; ++i)
        {
            int2 texIndex = _dispatchId.xy;
            texIndex.x = max(texIndex.x - (EDGE_SEARCH_STEP - i), 0);
            float value = RGBToLuminance(SourceTex[texIndex].rgb);
            int2 cacheIndex = int2(i, _groupId.y);
            LuminCache[Index2DTo1DEx(cacheIndex)] = value;
        }
    }
    else if (_groupId.x == 15)
    {
        [unroll]
        for (int i = 1; i <= EDGE_SEARCH_STEP; ++i)
        {
            int2 texIndex = _dispatchId.xy;
            texIndex.x = min(texIndex.x + i, 1280 - 1);
            float value = RGBToLuminance(SourceTex[texIndex].rgb);
            int2 cacheIndex = int2(i + 15 + EDGE_SEARCH_STEP, _groupId.y);
            LuminCache[Index2DTo1DEx(cacheIndex)] = value;
        }
    }

    if (_groupId.y == 0)
    {
        [unroll]
        for (int i = 0; i < EDGE_SEARCH_STEP; ++i)
        {
            int2 texIndex = _dispatchId.xy;
            texIndex.y = max(texIndex.y - (EDGE_SEARCH_STEP - i), 0);
            float value = RGBToLuminance(SourceTex[texIndex].rgb);
            int2 cacheIndex = int2(_groupId.x, i);
            LuminCache[Index2DTo1DEx(cacheIndex)] = value;
        }
    }
    else if (_groupId.y == 15)
    {
        [unroll]
        for (int i = 1; i <= EDGE_SEARCH_STEP; ++i)
        {
            int2 texIndex = _dispatchId.xy;
            texIndex.y = min(texIndex.x + i, 720 - 1);
            float value = RGBToLuminance(SourceTex[texIndex].rgb);
            int2 cacheIndex = int2(_groupId.x, i + 15 + EDGE_SEARCH_STEP);
            LuminCache[Index2DTo1DEx(cacheIndex)] = value;
        }
    }

    {
        int2 texIndex = _dispatchId.xy;
        float value = RGBToLuminance(SourceTex[texIndex].rgb);
        int2 cacheIndex = _groupId.xy + (int2)EDGE_SEARCH_STEP;
        LuminCache[Index2DTo1DEx(cacheIndex)] = value;
    }

    GroupMemoryBarrierWithGroupSync();

    int2 cindex = _groupId.xy + (int2)EDGE_SEARCH_STEP;
    float M = LuminCache[Index2DTo1DEx(cindex)];
    float N = LuminCache[Index2DTo1DEx(int2(cindex.x, cindex.y - 1))];
    float S = LuminCache[Index2DTo1DEx(int2(cindex.x, cindex.y + 1))];
    float W = LuminCache[Index2DTo1DEx(int2(cindex.x - 1, cindex.y))];
    float E = LuminCache[Index2DTo1DEx(int2(cindex.x + 1, cindex.y))];

    float maxLumin = max(max(max(N, S), max(W, E)), M);
    float minLumin = min(min(min(N, S), min(W, E)), M);
    float contrast = maxLumin - minLumin;
    float2 pixelStep = (float2)0.f;
    float pixelBlend = 0.f;

    bool needAA = contrast > max(MIN_EDGE_THRESHOLD, maxLumin * EDGE_THRESHOLD);

    float2 texelSize = float2(1.f / 1280.f, 1.f / 720.f);
    float2 cuv = float2(((float)_dispatchId.x + 0.5f) / 1280.f, ((float)_dispatchId.y + 0.5f) / 720.f);

    if (needAA)
    {
        float NW = LuminCache[Index2DTo1DEx(int2(cindex.x - 1, cindex.y - 1))];
        float NE = LuminCache[Index2DTo1DEx(int2(cindex.x + 1, cindex.y - 1))];
        float SW = LuminCache[Index2DTo1DEx(int2(cindex.x - 1, cindex.y + 1))];
        float SE = LuminCache[Index2DTo1DEx(int2(cindex.x + 1, cindex.y + 1))];

        float filter = (2.f * (N + W + S + E) + NW + NE + SW + SE) / 12.f;
        filter = saturate(abs(filter - M) / contrast);

        pixelBlend = smoothstep(0.f, 1.f, filter);
        pixelBlend *= pixelBlend;

        float vert = abs(N + S - 2.f * M) * 2.f + abs(NE + SE - 2.f * E) + abs(NW + SW - 2.f * W);
        float hori = abs(E + W - 2.f * M) * 2.f + abs(NE + NW - 2.f * N) + abs(SE + SW - 2.f * S);
        bool isHori = vert > hori;
        pixelStep = isHori ? float2(0, texelSize.y) : float2(texelSize.x, 0);
        float positive = abs((isHori ? N : E) - M);
        float negative = abs((isHori ? S : W) - M);
        if (positive < negative)
        {
            pixelStep *= -1.f;
        }
    }

    GroupMemoryBarrierWithGroupSync();

    if (needAA)
    {
        SourceTex[_dispatchId.xy] = CopyTex.SampleLevel(LinearBorder, cuv + pixelStep * pixelBlend, 0.f);
    }
}
