Texture2D<float4> gFilterPixel : register(t0);

RWTexture2D<float4> gUpSamplingTarget : register(u0);
RWTexture2D<float4> gUpSamplingSource : register(u1);

SamplerState gLinearBorder : register(s0);

cbuffer UP_SAMPLING_INFO : register(b0)
{
    uint gCurrentTexWidth;
    uint gCurrentTexHeight;
    uint gTargetMip;
    uint gPad[1];
}

static const float gFilterWeight[9] = 
{
    1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
    2.f / 16.f, 4.f / 16.f, 2.f / 16.f,
    1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
};

[numthreads(16, 16, 1)]
void main(int3 _dispatchId : SV_DispatchThreadID)
{
    float4 upValue = (float4)0.f;
    if (gTargetMip == 5)
    {
        float2 uv;
        uv.x = (_dispatchId.x + 0.5f) / gCurrentTexWidth;
        uv.y = (_dispatchId.y + 0.5f) / gCurrentTexHeight;
        upValue = gFilterPixel.SampleLevel(gLinearBorder, uv, gTargetMip + 2);
    }
    else
    {
        float4 sampleValueArray[9];
        {
            int2 centerXY = _dispatchId.xy / 2;
            int2 preMipMaxSize = int2(gCurrentTexWidth, gCurrentTexHeight) / 2 - 1;
            int2 index0 = int2(max(centerXY.x - 1, 0), max(centerXY.y - 1, 0));
            int2 index1 = int2(centerXY.x, max(centerXY.y - 1, 0));
            int2 index2 = int2(min(centerXY.x + 1, preMipMaxSize.x), max(centerXY.y - 1, 0));
            int2 index3 = int2(max(centerXY.x - 1, 0), centerXY.y);
            int2 index4 = int2(centerXY);
            int2 index5 = int2(min(centerXY.x + 1, preMipMaxSize.x), centerXY.y);
            int2 index6 = int2(max(centerXY.x - 1, 0), min(centerXY.y + 1, preMipMaxSize.y));
            int2 index7 = int2(centerXY.x, min(centerXY.y + 1, preMipMaxSize.y));
            int2 index8 = int2(min(centerXY.x + 1, preMipMaxSize.x), min(centerXY.y + 1, preMipMaxSize.y));
            sampleValueArray[0] = gUpSamplingSource[index0];
            sampleValueArray[1] = gUpSamplingSource[index1];
            sampleValueArray[2] = gUpSamplingSource[index2];
            sampleValueArray[3] = gUpSamplingSource[index3];
            sampleValueArray[4] = gUpSamplingSource[index4];
            sampleValueArray[5] = gUpSamplingSource[index5];
            sampleValueArray[6] = gUpSamplingSource[index6];
            sampleValueArray[7] = gUpSamplingSource[index7];
            sampleValueArray[8] = gUpSamplingSource[index8];
        }
        [unroll]
        for (int i = 0; i < 9; ++i)
        {
            upValue += gFilterWeight[i] * sampleValueArray[i];
        }
    }
    float4 addValue = gFilterPixel.Load(int3(_dispatchId.xy, gTargetMip + 1));

    gUpSamplingTarget[_dispatchId.xy] = upValue + addValue;
}
