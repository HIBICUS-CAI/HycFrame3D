#ifndef MIN_VALUE
#define MIN_VALUE (2.f)
#endif

RWTexture2D<float4> gBloomTex : register(u0);

Texture2D<float4> gOriginTex : register(t0);

groupshared float4 gLightCache[256];

[numthreads(16, 16, 1)]
void main(int3 _groupId : SV_GroupThreadID, int3 _dispatchId : SV_DispatchThreadID)
{
    int linearGroupIndex = _groupId.x * 16 + _groupId.y;
    gLightCache[linearGroupIndex] = (float4)0.f;

    float4 originValue = gOriginTex.Load(_dispatchId);
    if (originValue.r > MIN_VALUE || originValue.g > MIN_VALUE || originValue.b > MIN_VALUE)
    {
        gLightCache[linearGroupIndex] = float4(originValue.rgb, 1.f);
    }

    gBloomTex[_dispatchId.xy] = gLightCache[linearGroupIndex];
}
