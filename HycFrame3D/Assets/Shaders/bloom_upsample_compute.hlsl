Texture2D<float4> gFilterPixel : register(t0);

RWTexture2D<float4> gUpSamplingTarget : register(u0);

SamplerState gLinearClamp : register(s0);

cbuffer UP_SAMPLING_INFO : register(b0)
{
    uint gCurrentTexWidth;
    uint gCurrentTexHeight;
    uint gCurrentMip;
    uint gPad[1];
}

[numthreads(16, 16, 1)]
void main(int3 _dispatchId : SV_DispatchThreadID)
{
    float2 uv;
    uv.x = (_dispatchId.x + 0.5f) / gCurrentTexWidth;
    uv.y = (_dispatchId.y + 0.5f) / gCurrentTexHeight;
    float4 upValue = gFilterPixel.SampleLevel(gLinearClamp, uv, gCurrentMip);
    float4 addValue = gFilterPixel.Load(int3(_dispatchId.xy, gCurrentMip - 1));

    gUpSamplingTarget[_dispatchId.xy] = upValue + addValue;
}
