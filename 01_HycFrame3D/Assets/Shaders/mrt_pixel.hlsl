#include "gbuffer_utility.hlsli"
#include "light_disney_pbr.hlsli"

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float4 DiffuseAlbedo : COLOR0;
    uint3 MaterialIndexFactor : COLOR1;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexCoordL : TEXCOORD;
    uint4 UsePBRTex : BLENDINDICES;
    float EmissiveIntensity : BLENDINDICES4;
};

struct PS_OUTPUT
{
    uint4 GeoData : SV_Target0;
    float4 AnisotropicData : SV_Target1;
};

StructuredBuffer<MATERIAL> gAllMaterialInfo : register(t0);
Texture2D gAlbedo : register(t1);
Texture2D gBumped : register(t2);
Texture2D gMetallic : register(t3);
Texture2D gRoughness : register(t4);
Texture2D gEmissive : register(t5);

SamplerState gLinearSampler : register(s0);

float3 ClacBumpedNormal(float3 _normalMapSample,
    float3 _unitNormalW, float3 _tangentW)
{
    float3 normalT = 2.0f * _normalMapSample - 1.0f;
    float3 N = _unitNormalW;
    float3 T = normalize(_tangentW - dot(_tangentW, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);

    return mul(normalT, TBN);
}

PS_OUTPUT main(VS_OUTPUT _input)
{
    float4 albedoAndAlpha = gAlbedo.Sample(gLinearSampler,_input.TexCoordL);
    clip(albedoAndAlpha.a - 0.1f);

    float3 unitNormal = _input.NormalW;
    
    float3 tangent = normalize(_input.TangentW);
    float3 bitTangent = normalize(cross(normalize(unitNormal), tangent));
    float2 encodeTangent = EncodeNormalizeVec(tangent);
    float2 encodeBitTangent = EncodeNormalizeVec(bitTangent);

    if (_input.UsePBRTex.x == 1)
    {
        float3 noramlSample = gBumped.Sample(gLinearSampler, _input.TexCoordL).rgb;
        _input.NormalW = ClacBumpedNormal(noramlSample, unitNormal, _input.TangentW);
    }

    _input.NormalW = normalize(_input.NormalW);
    
    float2 encodeNormal = EncodeNormalizeVec(_input.NormalW);
    uint2 encodeUNormal = FloatToUint16_V2(encodeNormal);
    uint geoNormalData = PackTwoUint16ToUint32(encodeUNormal.x, encodeUNormal.y);
    
    uint4 alFctUint = FloatToUint8_V4(float4(albedoAndAlpha.rgb, asfloat(_input.MaterialIndexFactor.z)));
    uint geoAlbeAndFactor = PackFourUint8ToUint32(alFctUint.x, alFctUint.y, alFctUint.z, alFctUint.w);
    
    uint majorMatIndex = _input.MaterialIndexFactor.x;
    uint minorMatIndex = _input.MaterialIndexFactor.y;
    float factor = asfloat(_input.MaterialIndexFactor.z);
    MATERIAL m1 = gAllMaterialInfo[majorMatIndex];
    MATERIAL m2 = gAllMaterialInfo[minorMatIndex];
    float roughness = lerp(m1.mRoughness, m2.mRoughness, factor);
    float metallic = lerp(m1.mMetallic, m2.mMetallic, factor);

    if (_input.UsePBRTex.y == 1)
    {
        metallic = gMetallic.Sample(gLinearSampler, _input.TexCoordL);
    }
    if (_input.UsePBRTex.z == 1)
    {
        roughness = gRoughness.Sample(gLinearSampler, _input.TexCoordL);
    }

    uint4 matData = uint4(FloatToUint8_V2(float2(metallic, roughness)),
        majorMatIndex, minorMatIndex);
    uint geoMatData = PackFourUint8ToUint32(matData.x, matData.y, matData.z, matData.w);
    
    uint4 emissInten = (uint4)0;
    if (_input.UsePBRTex.w == 1 && _input.EmissiveIntensity != 0.f)
    {
        float3 emissive = gEmissive.Sample(gLinearSampler, _input.TexCoordL);
        float emissIntensityFloat = _input.EmissiveIntensity;
        float emissIntensityRatio = emissIntensityFloat / EMISSIVE_INTENSITY_MAX;
        emissIntensityRatio = saturate(emissIntensityRatio);
        emissInten = FloatToUint8_V4(float4(emissive.rgb, emissIntensityRatio));
    }

    uint geoEmiss = PackFourUint8ToUint32(emissInten.x, emissInten.y, emissInten.z, emissInten.w);

    PS_OUTPUT _out = (PS_OUTPUT)0;
    _out.GeoData = uint4(geoNormalData, geoAlbeAndFactor, geoMatData, geoEmiss);
    _out.AnisotropicData = float4(encodeTangent, encodeBitTangent);
    
    return _out;
}
