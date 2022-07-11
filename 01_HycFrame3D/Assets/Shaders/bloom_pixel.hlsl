#include "color_utility.hlsli"

#ifndef PIXEL_FACTOR
#define PIXEL_FACTOR (0.02f)
#endif

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float4 LightColorW : COLOR;
};

float4 main(VS_OUTPUT _input) : SV_TARGET
{
#ifdef BLOOM_ON
    _input.LightColorW.rgb *= _input.LightColorW.a * PIXEL_FACTOR;
#endif
    _input.LightColorW = float4(sRGBToACES(_input.LightColorW.rgb), 1.f);
    return _input.LightColorW;
}
