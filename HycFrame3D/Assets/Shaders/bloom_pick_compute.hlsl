RWTexture2D<float4> gBloomTex : register(u0);

Texture2D<float4> gOriginTex : register(t0);

groupshared float4 gLightCache[256];

[numthreads(16, 16, 1)]
void main(int3 _groupId : SV_GroupThreadID, int3 _dispatchId : SV_DispatchThreadID)
{
    int linearGroupIndex = _groupId.x * 16 + _groupId.y;
    gLightCache[linearGroupIndex] = (float4)0.f;

    float4 originValue = gOriginTex.Load(_dispatchId);
    if (originValue.r > 2.f || originValue.g > 2.f || originValue.b > 2.f)
    {
        gLightCache[linearGroupIndex] = float4(originValue.rgb, 1.f);
    }

    gBloomTex[_dispatchId.xy] = gLightCache[linearGroupIndex];
}
