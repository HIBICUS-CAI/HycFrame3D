#if !defined BRDF_DISNEY && !defined BLINN_PHONG
#define BRDF_DISNEY
#endif

#if defined BRDF_DISNEY
#include "light_disney_pbr.hlsli"
#elif defined BLINN_PHONG
#include "light_blinn_phong.hlsli"
#endif

#include "color_utility.hlsli"
#include "gbuffer_utility.hlsli"

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoordL : TEXCOORD;
};

struct AMBIENT
{
    float4 gAmbientFactor;
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

struct VIEWPROJ
{
    matrix gInvView;
    matrix gInvProj;
};

SamplerState gSamPointClamp : register(s0);
SamplerState gSamLinearWrap : register(s1);
SamplerComparisonState gSamShadowCom : register(s2);

StructuredBuffer<AMBIENT> gAmbient : register(t0);
StructuredBuffer<LIGHT_INFO> gLightInfo : register(t1);
StructuredBuffer<LIGHT> gLights : register(t2);
StructuredBuffer<SHADOW_INFO> gShadowInfo : register(t3);
StructuredBuffer<VIEWPROJ> gInvCameraInfo : register(t4);
StructuredBuffer<MATERIAL> gAllMaterialInfo : register(t5);

Texture2D<uint4> gGeoBuffer : register(t6);
Texture2D gAnisotropic : register(t7);
Texture2D gSsao : register(t8);
Texture2DArray<float> gShadowMap : register(t9);
Texture2D gBRDFLUT : register(t10);
TextureCube gDiffuseMap : register(t11);
TextureCube gSpecularMap : register(t12);
Texture2D gDepthMap : register(t13);

float3 DepthToWorldPos(float2 uv, float depth)
{
    float4 ndc = float4(uv * 2.f - 1.f, depth, 1.f);
	ndc.y *= -1.f;
	float4 wp = mul(ndc, gInvCameraInfo[0].gInvProj);
    wp = mul(wp, gInvCameraInfo[0].gInvView);
	return (wp / wp.w).xyz;
}

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

MATERIAL LerpMaterial(MATERIAL _m1, MATERIAL _m2, float _factor)
{
    MATERIAL final;
    final.mFresnelR0 = _m1.mFresnelR0;
    final.mSubSruface = lerp(_m1.mSubSruface, _m2.mSubSruface, _factor);
    final.mMetallic = lerp(_m1.mMetallic, _m2.mMetallic, _factor);
    final.mSpecular = lerp(_m1.mSpecular, _m2.mSpecular, _factor);
    final.mSpecularTint = lerp(_m1.mSpecularTint, _m2.mSpecularTint, _factor);
    final.mRoughness = lerp(_m1.mRoughness, _m2.mRoughness, _factor);
    final.mAnisotropic = lerp(_m1.mAnisotropic, _m2.mAnisotropic, _factor);
    final.mSheen = lerp(_m1.mSheen, _m2.mSheen, _factor);
    final.mSheenTint = lerp(_m1.mSheenTint, _m2.mSheenTint, _factor);
    final.mClearcoat = lerp(_m1.mClearcoat, _m2.mClearcoat, _factor);
    final.mClearcoatGloss = lerp(_m1.mClearcoatGloss, _m2.mClearcoatGloss, _factor);

    return final;
}

float3 LerpFresnelR0(float3 _originR0, float _metallic)
{
    if (_metallic < 1.f && _originR0.x > 0.1f && _originR0.y > 0.1f && _originR0.z > 0.1f)
    {
        float minV = min(min(_originR0.x, _originR0.y), _originR0.z);
        float factor = 1.f - Pow5(_metallic);
        return lerp(float3(minV, minV, minV), float3(0.1f, 0.1f, 0.1f), factor);
    }
    else
    {
        return _originR0;
    }
}

float3 CalcSpecLookUpVec(float3 _pos, float3 _normal)
{
    const float3 BOX_AABB_SIZE = float3(10000.f, 10000.f, 10000.f);
    float3 p = _pos - gLightInfo[0].gCameraPos;
    float3 unitRayDir = normalize(reflect(p, _normal));
    float3 t1 = (-p + BOX_AABB_SIZE) / unitRayDir;
    float3 t2 = (-p - BOX_AABB_SIZE) / unitRayDir;
    float3 tmax = max(t1, t2);
    float t = min(min(tmax.x, tmax.y), tmax.z);
    float3 lookBoxVec = p + t * unitRayDir;

    return lookBoxVec;
}

float3 FresnelSchlick_Roughness(float _cosTheta, float3 _f0, float _roughness)
{
    return _f0 + (max((float3)(1.f - _roughness), _f0) - _f0) * Pow5(1.f - _cosTheta);
}

float3 CalcEnvDiffuse(float3 _normal, MATERIAL _mat, float3 _view)
{
    float3 kS = FresnelSchlick_Roughness(max(dot(_normal, _view), 0.f), _mat.mFresnelR0, _mat.mRoughness);
    float3 kD = 1.f - kS;
    float3 irradiance = gDiffuseMap.Sample(gSamLinearWrap, _normal).rgb;
    return (1.f - _mat.mMetallic) * kD * irradiance;
}

float3 CalcEnvSpecular(float3 _pos, float3 _normal ,float3 _view, MATERIAL _mat)
{
    float3 lookUpVec = CalcSpecLookUpVec(_pos, _normal);
    const float MAX_LOD = 9.f;
    float3 preFilteredColor = gSpecularMap.SampleLevel(gSamLinearWrap, lookUpVec,
        _mat.mRoughness * MAX_LOD).rgb;
    float NdotV = max(dot(_normal, _view), 0.f);
    float3 F = FresnelSchlick_Roughness(NdotV, _mat.mFresnelR0, _mat.mRoughness);
    float2 envBRDF = gBRDFLUT.Sample(gSamLinearWrap, float2(NdotV, _mat.mRoughness)).rg;
    float3 specular = preFilteredColor * (F + envBRDF.r + envBRDF.g);

    return specular;
}

float4 main(VS_OUTPUT _in) : SV_TARGET
{
    float depth = gDepthMap.Sample(gSamPointClamp, _in.TexCoordL).r;
    float3 positionW = DepthToWorldPos(_in.TexCoordL, depth);
    int3 tcInt = int3(_in.TexCoordL.x * 1280, _in.TexCoordL.y * 720, 0);
    uint4 geoData = gGeoBuffer.Load(tcInt);
    float4 anisoData = gAnisotropic.Sample(gSamPointClamp, _in.TexCoordL);
    float3 normalW = GetNormalFromGeoValue(geoData.x);
    float3 anisoX = DecodeNormalizeVec(anisoData.xy);
    float3 anisoY = DecodeNormalizeVec(anisoData.zw);
    float3 foedebug = anisoX + anisoY;
    float4 albedoAndMatFactor = Uint8ToFloat_V4(UnpackUint32ToFourUint8(geoData.y));
    uint4 matAbout = UnpackUint32ToFourUint8(geoData.z);
    float2 metAndRou = Uint8ToFloat_V2(matAbout.xy);
    uint4 emissAbout = UnpackUint32ToFourUint8(geoData.w);
    float4 emissIntensity = Uint8ToFloat_V4(emissAbout);
    float3 emissive = (float3)0.f;
    float3 toEye = normalize(gLightInfo[0].gCameraPos - positionW);
    float4 diffuse = float4(albedoAndMatFactor.rgb, 1.f);
    float access = gSsao.SampleLevel(gSamLinearWrap, _in.TexCoordL, 0.0f).r;
    diffuse.rgb = sRGBToACES(diffuse.rgb);
    MATERIAL mat = LerpMaterial(gAllMaterialInfo[matAbout.z],
        gAllMaterialInfo[matAbout.w], albedoAndMatFactor.w);
    mat.mRoughness = metAndRou.y;
    mat.mMetallic = metAndRou.x;
    mat.mFresnelR0 = LerpFresnelR0(mat.mFresnelR0, mat.mMetallic);

    float3 envDiffuse = CalcEnvDiffuse(normalW, mat, toEye) * gAmbient[0].gAmbientFactor.rgb;
    float4 ambientL = float4(envDiffuse * access, 0.f);
    // ambientL = gAmbient[0].gAmbient * albedo * access;

    if (emissAbout.x != 0 && emissAbout.y != 0 && emissAbout.z != 0 && emissAbout.w != 0)
    {
        emissive = sRGBToACES(emissIntensity.rgb) * emissIntensity.a * EMISSIVE_INTENSITY_MAX;
    }

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
        LIGHT l = gLights[i];
        l.gAlbedo = sRGBToACES(l.gAlbedo);
#if defined BRDF_DISNEY
        tempL = float4(ComputeDirectionalLight(diffuse.rgb, l, mat, normalW, toEye, anisoX, anisoY), 0.0f);
#elif defined BLINN_PHONG
        tempL = float4(ComputeDirectionalLight(diffuse.rgb, l, mat, normalW, toEye), 0.0f);
#endif
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
        LIGHT l = gLights[i];
        l.gAlbedo = sRGBToACES(l.gAlbedo);
#if defined BRDF_DISNEY
        directL += float4(ComputePointLight(diffuse.rgb, l, mat, positionW, normalW, toEye, anisoX, anisoY), 0.0f);
#elif defined BLINN_PHONG
        directL += float4(ComputePointLight(diffuse.rgb, l, mat, positionW, normalW, toEye), 0.0f);
#endif
    }

    for (i = dNum + pNum; i < dNum + pNum + sNum; ++i)
    {
        LIGHT l = gLights[i];
        l.gAlbedo = sRGBToACES(l.gAlbedo);
#if defined BRDF_DISNEY
        tempL = float4(ComputeSpotLight(diffuse.rgb, l, mat, positionW, normalW, toEye, anisoX, anisoY), 0.0f);
#elif defined BLINN_PHONG
        tempL = float4(ComputeSpotLight(diffuse.rgb, l, mat, positionW, normalW, toEye), 0.0f);
#endif
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

    directL += lerp(0.1f, 1.f, mat.mMetallic) *
        float4(CalcEnvSpecular(positionW, normalW, toEye, mat) * diffuse.rgb, 0.f) *
        gAmbient[0].gAmbientFactor;

    float4 litColor = ambientL * diffuse + directL + float4(emissive, 0.f);
    litColor.a = diffuse.a;

    return litColor;
}
