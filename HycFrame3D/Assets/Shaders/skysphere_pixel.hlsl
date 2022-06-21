#include "color_utility.hlsli"

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

TextureCube gCubeMap : register(t0);

SamplerState gSamLinearWrap : register(s0);

float4 main(VS_OUTPUT _input) : SV_TARGET
{
    float4 cubeColor = gCubeMap.Sample(gSamLinearWrap, _input.PosL);
    cubeColor.rgb = sRGBToACES(cubeColor.rgb);
    return cubeColor;
}
