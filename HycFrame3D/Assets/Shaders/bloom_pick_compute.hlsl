RWTexture2D<float4> gBloomTex : register(u0);

Texture2D<float4> gOriginTex : register(t0);

groupshared float4 gLightCache[256];

[numthreads(16, 16, 1)]
void main(int3 _groupId : SV_GroupThreadID, int3 _dispatchId : SV_DispatchThreadID)
{
    int linearGroupIndex = _groupId.x * 16 + _groupId.y;
    gLightCache[linearGroupIndex] = (float4)0.f;

    float4 originValue = gOriginTex.Load(_dispatchId);
    if (length(originValue) > 4.f)
    {
        gLightCache[linearGroupIndex] = originValue;
    }
}
