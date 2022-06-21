#include "color_utility.hlsli"

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float4 LightColorW : COLOR;
};

float4 main(VS_OUTPUT _input) : SV_TARGET
{
    _input.LightColorW.rgb *= _input.LightColorW.a;
    _input.LightColorW = float4(sRGBToACES(_input.LightColorW.rgb), 1.f);
    return _input.LightColorW;
}
