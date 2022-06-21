#include "color_utility.hlsli"

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoordL : TEXCOORD;
};

SamplerState gSamLinearWrap : register(s0);

Texture2D gACESTexture : register(t0);

float4 main(VS_OUTPUT _in) : SV_TARGET
{
    float4 aces = gACESTexture.Sample(gSamLinearWrap, _in.TexCoordL);

    return aces;
}
