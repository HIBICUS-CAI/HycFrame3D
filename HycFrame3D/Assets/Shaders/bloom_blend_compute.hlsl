Texture2D<float4> gBloomResult : register(t0);

RWTexture2D<float4> gHdrTexture : register(u0);

SamplerState gLinearClamp : register(s0);

[numthreads(16, 16, 1)]
void main(int3 _dispatchId : SV_DispatchThreadID)
{
    float2 uv;
    uv.x = (_dispatchId.x + 0.5f) / 1280.f;
    uv.y = (_dispatchId.y + 0.5f) / 720.f;
    
    float4 originValue = gHdrTexture[_dispatchId.xy];
    float4 bloomValue = gBloomResult.SampleLevel(gLinearClamp, uv, 0);

    float alpha = saturate(bloomValue.a);
    originValue.rgb = originValue.rgb * (1.f - alpha) + bloomValue.rgb * alpha;
    originValue.a = 1.f;

    gHdrTexture[_dispatchId.xy] = originValue;
}
