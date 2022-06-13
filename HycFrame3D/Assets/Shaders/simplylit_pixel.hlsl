#include "gbuffer_utility.hlsli"

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoordL : TEXCOORD;
};

SamplerState gSamLinearWrap : register(s0);

Texture2D<uint4> gGeoBuffer : register(t0);
Texture2D gSsao : register(t1);

float4 main(VS_OUTPUT _in) : SV_TARGET
{
    int3 tcInt = int3(_in.TexCoordL.x * 1280, _in.TexCoordL.y * 720, 0);
    uint4 geoData = gGeoBuffer.Load(tcInt);
    float4 geoAlbedo = float4(Uint8ToFloat_V4(UnpackUint32ToFourUint8(geoData.y)).rgb, 0.f);
    float access = gSsao.SampleLevel(gSamLinearWrap, _in.TexCoordL, 0.0f).r;
    access = access * access;

    float4 final = access * geoAlbedo;
    final.a = 1.f;

    return final;
}
