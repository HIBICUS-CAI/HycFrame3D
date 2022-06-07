#include "light_disney_pbr.hlsli"
#include "color_utility.hlsli"

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoordL : TEXCOORD;
};

struct AMBIENT
{
    float4 gAmbient;
};

struct LIGHT_INFO
{
    float3 gCameraPos;
    float gPad0;
    uint gDirectLightNum;
    uint gSpotLightNum;
    uint gPointLightNum;
    uint gShadowLightNum;
    int gShadowLightIndex[4];
};

struct SHADOW_INFO
{
    matrix gShadowViewMat;
    matrix gShadowProjMat;
    matrix gSSAOMat;
};

SamplerState gSamPointClamp : register(s0);
SamplerState gSamLinearWrap : register(s1);
SamplerComparisonState gSamShadowCom : register(s2);

StructuredBuffer<AMBIENT> gAmbient : register(t0);
StructuredBuffer<LIGHT_INFO> gLightInfo : register(t1);
StructuredBuffer<LIGHT> gLights : register(t2);
StructuredBuffer<SHADOW_INFO> gShadowInfo : register(t3);

Texture2D gWorldPos : register(t4);
Texture2D gNormal : register(t5);
Texture2D gDiffuse : register(t6);
Texture2D gDiffuseAlbedo : register(t7);
Texture2D gFresnelShiniese : register(t8);
Texture2D gSsao : register(t9);
Texture2DArray<float> gShadowMap : register(t10);
TextureCube gCubeMap : register(t11);

float CalcShadowFactor(float4 _shadowPosH, float _slice)
{
    _shadowPosH.xyz /= _shadowPosH.w;
    _shadowPosH.x = 0.5f * _shadowPosH.x + 0.5f;
    _shadowPosH.y = -0.5f * _shadowPosH.y + 0.5f;

    uint width, height, numMips, elements;
    gShadowMap.GetDimensions(0, width, height, elements, numMips);

    const float WIDTH = (float)width;
    const float DX = 1.0f / WIDTH;
    const float HEIGHT = (float)height;
    const float DY = 1.0f / HEIGHT;

    float depth = _shadowPosH.z;
    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-DX,  -DY), float2(0.0f,  -DY), float2(DX,  -DY),
        float2(-DX, 0.0f), float2(0.0f, 0.0f), float2(DX, 0.0f),
        float2(-DX,  +DY), float2(0.0f,  +DY), float2(DX,  +DY)
    };
    
    float3 pos = float3(_shadowPosH.xy, _slice);
    [unroll]
    for(int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gSamShadowCom,
            pos + float3(offsets[i], 0.0f), depth).r;
    }

    return percentLit / 9.0f;
}

float4 main(VS_OUTPUT _in) : SV_TARGET
{
    float3 positionW = gWorldPos.Sample(gSamPointClamp, _in.TexCoordL).rgb;
    float3 normalW = normalize(gNormal.Sample(gSamPointClamp, _in.TexCoordL).rgb);
    float3 toEye = normalize(gLightInfo[0].gCameraPos - positionW);
    float4 albedo = gDiffuseAlbedo.Sample(gSamLinearWrap, _in.TexCoordL);
    float4 fresshin = gFresnelShiniese.Sample(gSamLinearWrap, _in.TexCoordL);
    float3 fresnel = fresshin.rgb;
    float shiniese = fresshin.a;
    float4 diffuse = gDiffuse.Sample(gSamLinearWrap, _in.TexCoordL);
    float access = gSsao.SampleLevel(gSamLinearWrap, _in.TexCoordL, 0.0f).r;
    MATERIAL mat = (MATERIAL)0.0f;
    mat.mFresnelR0 = fresnel;
    mat.mRoughness = 1.f - shiniese;
    mat.mMetallic = 0.95f;
    mat.mSpecular = 0.8f;

    float4 ambientL = gAmbient[0].gAmbient * albedo * access;

    float4 directL = (float4)0.0f;
    float4 tempL = (float4)0.0f;
    float shadow = 0.0f;
    float4 shadowPosH = (float4)0.0f;
    uint i = 0;
    uint j = 0;
    uint dNum = gLightInfo[0].gDirectLightNum;
    uint pNum = gLightInfo[0].gPointLightNum;
    uint sNum = gLightInfo[0].gSpotLightNum;

    for (i = 0; i < dNum; ++i)
    {
        tempL = float4(ComputeDirectionalLight(diffuse.rgb, gLights[i], mat, normalW, toEye), 0.0f);
        if (i == gLightInfo[0].gShadowLightIndex[0])
        {
            shadowPosH = mul(float4(positionW, 1.0f), gShadowInfo[0].gShadowViewMat);
            shadowPosH = mul(shadowPosH, gShadowInfo[0].gShadowProjMat);
            shadow = CalcShadowFactor(shadowPosH, 0.0f);
            tempL.xyz *= shadow / gLightInfo[0].gShadowLightNum;
        }
        else if (i == gLightInfo[0].gShadowLightIndex[1])
        {
            shadowPosH = mul(float4(positionW, 1.0f), gShadowInfo[1].gShadowViewMat);
            shadowPosH = mul(shadowPosH, gShadowInfo[1].gShadowProjMat);
            shadow = CalcShadowFactor(shadowPosH, 1.0f);
            tempL.xyz *= shadow / gLightInfo[0].gShadowLightNum;
        }
        else if (i == gLightInfo[0].gShadowLightIndex[2])
        {
            shadowPosH = mul(float4(positionW, 1.0f), gShadowInfo[2].gShadowViewMat);
            shadowPosH = mul(shadowPosH, gShadowInfo[2].gShadowProjMat);
            shadow = CalcShadowFactor(shadowPosH, 2.0f);
            tempL.xyz *= shadow / gLightInfo[0].gShadowLightNum;
        }
        else if (i == gLightInfo[0].gShadowLightIndex[3])
        {
            shadowPosH = mul(float4(positionW, 1.0f), gShadowInfo[3].gShadowViewMat);
            shadowPosH = mul(shadowPosH, gShadowInfo[3].gShadowProjMat);
            shadow = CalcShadowFactor(shadowPosH, 3.0f);
            tempL.xyz *= shadow / gLightInfo[0].gShadowLightNum;
        }
        directL += tempL;
    }

    for (i = dNum; i < dNum + pNum; ++i)
    {
        directL += float4(ComputePointLight(diffuse.rgb, gLights[i], mat, positionW, normalW, toEye), 0.0f);
    }

    for (i = dNum + pNum; i < dNum + pNum + sNum; ++i)
    {
        tempL = float4(ComputeSpotLight(diffuse.rgb, gLights[i], mat, positionW, normalW, toEye), 0.0f);
        if (i == gLightInfo[0].gShadowLightIndex[0])
        {
            shadowPosH = mul(float4(positionW, 1.0f), gShadowInfo[0].gShadowViewMat);
            shadowPosH = mul(shadowPosH, gShadowInfo[0].gShadowProjMat);
            shadow = CalcShadowFactor(shadowPosH, 0.0f);
            tempL.xyz *= shadow / gLightInfo[0].gShadowLightNum;
        }
        else if (i == gLightInfo[0].gShadowLightIndex[1])
        {
            shadowPosH = mul(float4(positionW, 1.0f), gShadowInfo[1].gShadowViewMat);
            shadowPosH = mul(shadowPosH, gShadowInfo[1].gShadowProjMat);
            shadow = CalcShadowFactor(shadowPosH, 1.0f);
            tempL.xyz *= shadow / gLightInfo[0].gShadowLightNum;
        }
        else if (i == gLightInfo[0].gShadowLightIndex[2])
        {
            shadowPosH = mul(float4(positionW, 1.0f), gShadowInfo[2].gShadowViewMat);
            shadowPosH = mul(shadowPosH, gShadowInfo[2].gShadowProjMat);
            shadow = CalcShadowFactor(shadowPosH, 2.0f);
            tempL.xyz *= shadow / gLightInfo[0].gShadowLightNum;
        }
        else if (i == gLightInfo[0].gShadowLightIndex[3])
        {
            shadowPosH = mul(float4(positionW, 1.0f), gShadowInfo[3].gShadowViewMat);
            shadowPosH = mul(shadowPosH, gShadowInfo[3].gShadowProjMat);
            shadow = CalcShadowFactor(shadowPosH, 3.0f);
            tempL.xyz *= shadow / gLightInfo[0].gShadowLightNum;
        }
        directL += tempL;
    }

    // TEMP SIMPLY IBL
    float skyBoxEdgeLength = 10000.f;
    float3 boxExtents = (float3)skyBoxEdgeLength;
    float3 p = positionW - gLightInfo[0].gCameraPos;
    float3 unitRayDir = normalize(reflect(p, normalW));
    float3 t1 = (-p + boxExtents) / unitRayDir;
    float3 t2 = (-p - boxExtents) / unitRayDir;
    float3 tmax = max(t1, t2);
    float t = min(min(tmax.x, tmax.y), tmax.z);
    float3 lookBoxVec = p + t * unitRayDir;
    float4 boxColor = gCubeMap.Sample(gSamLinearWrap, lookBoxVec);
    float metalFactor = lerp(0.2f, 1.f, mat.mMetallic);
    float f0 = 1.f - saturate(dot(normalW, unitRayDir));
    f0 = Pow5(f0);
    float3 frsnFactor = fresnel + (1.f - fresnel) * f0;
    directL += boxColor * metalFactor * float4(MonToLin(diffuse.rgb), 0.f) * (1.f - mat.mRoughness) * float4(frsnFactor, 0.f);
    // TEMP SIMPLY IBL

    float4 litColor = ambientL * diffuse + directL;
    litColor.rgb = pow(litColor.rgb, float3(1.f / 2.2f, 1.f / 2.2f, 1.f / 2.2f));
    litColor.a = diffuse.a;

    return litColor;
}
