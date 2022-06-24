cbuffer BLUR_INFO : register(b0)
{
    uint gTexWidth;
    uint gTexHeight;
    uint gPads[2];
}

RWTexture2D<float4> gMipUav : register(u0);

groupshared float4 gColorCache[256 + 2 * 2];

static const float gBlurWeight[5] = { 0.0545f, 0.2442f, 0.4026f, 0.2442f, 0.0545f };

[numthreads(256, 1, 1)]
void HMain(int3 _groupId : SV_GroupThreadID, int3 _dispatchId : SV_DispatchThreadID)
{
    if (_groupId.x < 2)
    {
        int x = max(_dispatchId.x - 2, 0);
        gColorCache[_groupId.x] = gMipUav[int2(x, _dispatchId.y)];
    }
    if (_groupId.x >= 256 - 2)
    {
        int x = min(_dispatchId.x + 2, gTexWidth - 1);
        gColorCache[_groupId.x + 2 * 2] = gMipUav[int2(x, _dispatchId.y)];
    }
    gColorCache[_groupId.x + 2] = gMipUav[min(_dispatchId.xy, int2(gTexWidth, gTexHeight) - 1)];

    GroupMemoryBarrierWithGroupSync();

    float4 blur = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;

    for (int i = -2; i <= 2; ++i)
    {
        int texIndex = _groupId.x + 2 + i;
        blur += gBlurWeight[i + 2] * gColorCache[texIndex];
        totalWeight += gBlurWeight[i + 2];       
    }

    blur /= totalWeight;

    gMipUav[_dispatchId.xy] = blur;
}

[numthreads(1, 256, 1)]
void VMain(int3 _groupId : SV_GroupThreadID, int3 _dispatchId : SV_DispatchThreadID)
{
    if (_groupId.y < 2)
    {
        int y = max(_dispatchId.y - 2, 0);
        gColorCache[_groupId.y] = gMipUav[int2(_dispatchId.x, y)];
    }
    if (_groupId.y >= 256 - 2)
    {
        int y = min(_dispatchId.y + 2, gTexHeight - 1);
        gColorCache[_groupId.y + 2 * 2] = gMipUav[int2(_dispatchId.x, y)];
    }
    gColorCache[_groupId.y + 2] = gMipUav[min(_dispatchId.xy, int2(gTexWidth, gTexHeight) - 1)];

    GroupMemoryBarrierWithGroupSync();

    float4 blur = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;

    for (int i = -2; i <= 2; ++i)
    {
        int texIndex = _groupId.y + 2 + i;
        blur += gBlurWeight[i + 2] * gColorCache[texIndex];
        totalWeight += gBlurWeight[i + 2];
    }

    blur /= totalWeight;

    gMipUav[_dispatchId.xy] = blur;
}
