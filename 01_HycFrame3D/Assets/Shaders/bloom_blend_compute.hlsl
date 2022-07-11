cbuffer BLOOM_INTENSITY_INFO : register(b0)
{
    float gIntensityFactor;
    float gPads[3];
}

Texture2D<float4> gBloomResult : register(t0);

RWTexture2D<float4> gHdrTexture : register(u0);

SamplerState gLinearBorder : register(s0);

static const float gFilterWeight[9] = 
{
    1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
    2.f / 16.f, 4.f / 16.f, 2.f / 16.f,
    1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
};

[numthreads(16, 16, 1)]
void main(int3 _dispatchId : SV_DispatchThreadID)
{
    float2 uv;
    uv.x = (_dispatchId.x + 0.5f) / 1280.f;
    uv.y = (_dispatchId.y + 0.5f) / 720.f;
    float2 offset;
    offset.x = 1.f / 1280.f * 2.f;
    offset.y = 1.f / 720.f * 2.f;
    
    float4 originValue = gHdrTexture[_dispatchId.xy];
    float4 bloomValueA[9];
    bloomValueA[0] = gBloomResult.SampleLevel(gLinearBorder, uv - offset, 0);
    bloomValueA[1] = gBloomResult.SampleLevel(gLinearBorder, float2(uv.x, uv.y - offset.y), 0);
    bloomValueA[2] = gBloomResult.SampleLevel(gLinearBorder, float2(uv.x + offset.x, uv.y - offset.y), 0);
    bloomValueA[3] = gBloomResult.SampleLevel(gLinearBorder, float2(uv.x - offset.x, uv.y), 0);
    bloomValueA[4] = gBloomResult.SampleLevel(gLinearBorder, uv, 0);
    bloomValueA[5] = gBloomResult.SampleLevel(gLinearBorder, float2(uv.x + offset.x, uv.y), 0);
    bloomValueA[6] = gBloomResult.SampleLevel(gLinearBorder, float2(uv.x - offset.x, uv.y + offset.y), 0);
    bloomValueA[7] = gBloomResult.SampleLevel(gLinearBorder, float2(uv.x, uv.y + offset.y), 0);
    bloomValueA[8] = gBloomResult.SampleLevel(gLinearBorder, uv + offset, 0);

    float4 bloomValue = (float4)0.f;
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        bloomValue += gFilterWeight[i] * bloomValueA[i];
    }

    float alpha = saturate(bloomValue.a);
    originValue.rgb = originValue.rgb * (1.f - alpha) + bloomValue.rgb * gIntensityFactor * alpha;
    originValue.a = 1.f;

    gHdrTexture[_dispatchId.xy] = originValue;
}
