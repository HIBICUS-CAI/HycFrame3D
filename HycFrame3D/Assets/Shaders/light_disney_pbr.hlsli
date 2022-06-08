struct LIGHT
{
    float gTempIntensity;
    float3 gAlbedo;
    float gFalloffStart;
    float3 gDirection;
    float gFalloffEnd;
    float3 gPosition;
    float gSpotPower;
};

struct MATERIAL
{
    float3 mFresnelR0;
    float mSubSruface;
    float mMetallic;
    float mSpecular;
    float mSpecularTint;
    float mRoughness;
    float mAnisotropic;
    float mSheen;
    float mSheenTint;
    float mClearcoat;
    float mClearcoatGloss;
};

#define PI (3.14159265358979323846f)

float Sqr(float _val)
{
    return _val * _val;
}

float Pow5(float _val)
{
    float v2 = _val * _val;
    return v2 * v2 * _val;
}

float F_SchlickFresnel(float _HdotV)
{
    float m = saturate(1.f - _HdotV);
    return Pow5(m);
}

// alpha : roughness
float D_GTR1(float _NdotH, float _alpha)
{
    if (_alpha >= 1.f)
    {
        return 1.f / PI;
    }

    float a2 = Sqr(_alpha);
    float t = 1.f + (a2 - 1.f) * Sqr(_NdotH);

    return (a2 - 1.f) / (PI * log(a2) * t);
}

float D_GTR2(float _NdotH, float _alpha)
{
    float a2 = Sqr(_alpha);
    float t = 1.f + (a2 - 1.f) * Sqr(_NdotH);

    return a2 / (PI * Sqr(t));
}

float G_Smith_GGX(float _NdotV, float _alphaG)
{
    float aG2 = Sqr(_alphaG);
    float NdotV2 = Sqr(_NdotV);
    return 1.f / (_NdotV + sqrt(aG2 + NdotV2 - aG2 * NdotV2));
}

float3 Disney_BRDF(float3 _L, float3 _V, float3 _N, float3 _LStr, float3 _baseColor, MATERIAL _material)
{
    float NdotV = dot(_N, _V);
    float NdotL = dot(_N, _L);
    if (NdotL < 0.f || NdotV < 0.f)
    {
        return (float3)0.f;
    }

    float3 H = normalize(_L + _V);
    float NdotH = dot(_N, H);
    float LdotH = dot(_L, H);

    float3 cDLin = _baseColor;
    float cDLum = 0.3f * cDLin.x + 0.6f * cDLin.y + 0.1f * cDLin.z;
    float3 cTint = (cDLum > 0.f) ? (cDLin / cDLum) : (float3)1.f;
    float3 cSpec0 = lerp(_material.mSpecular * 0.08f *
        lerp((float3)1.f, cTint, _material.mSpecularTint),
        cDLin, _material.mMetallic);
    // float3 cSheen = lerp(float3(1.f), cTint, _material.mSheenTint);

    // Diffuse fresnel
    float fL = F_SchlickFresnel(NdotL);
    float fV = F_SchlickFresnel(NdotV);
    float fD90 = 0.5f + 2.f * Sqr(LdotH) * _material.mRoughness;
    float fD = lerp(1.f, fD90, fL) * lerp(1.f, fD90, fV);
    float fSS90 = Sqr(LdotH) * _material.mRoughness;
    float fSS = lerp(1.f, fSS90, fL) * lerp(1.f, fSS90, fV);
    float ss = 1.25f * (fSS * (1.f / (NdotL + NdotV) - 0.5f) + 0.5f);

    // Specular
    // float aspect = sqrt(1.f - _material.mAnisotropic * 0.9f);
    // float ax = max(0.001f, Sqr(_material.mRoughness) / aspect);
    // float ay = max(0.001f, Sqr(_material.mRoughness) * aspect);
    float dS = D_GTR2(NdotH, _material.mRoughness);
    float fH = F_SchlickFresnel(LdotH);
    float3 fS = lerp(cSpec0, (float3)1.f, fH);
    float gS = G_Smith_GGX(NdotL, _material.mRoughness) *
        G_Smith_GGX(NdotV, _material.mRoughness);
    
    // Sheen
    // float3 fSheen = fH * _material.mSheen * cSheen;

    // clearcoat (ior = 1.5, F0 = 0.04, roughness = 0.25)
    float dR = D_GTR1(NdotH, lerp(0.1f, 0.001f, _material.mClearcoatGloss));
    float fR = lerp(0.04f, 1.f, fH);
    float gR = G_Smith_GGX(NdotL, 0.25f) * G_Smith_GGX(NdotV, 0.25f);

    return _LStr * ((1.f / PI) *
        lerp(fD, ss, _material.mSubSruface) * cDLin) *
        (1.f - _material.mMetallic) +
        _LStr * gS * fS * dS +
        _LStr * 0.25f * _material.mClearcoat * gR * fR * dR;
}

float3 ComputeDirectionalLight(float3 baseColor, LIGHT l, MATERIAL mat, float3 normal, float3 toEye)
{
    float3 lightVec = -l.gDirection;
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStr = l.gAlbedo * l.gTempIntensity * ndotl;

    return Disney_BRDF(lightVec, toEye, normal, lightStr, baseColor, mat);
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    float EPSILON = 0.01;
    float att = 1.f / (d * d + EPSILON);
    float smoothatt = 1.f - pow(d / falloffEnd, 4);
    smoothatt = max(smoothatt, 0.f);
    smoothatt =  smoothatt * smoothatt;
    return att * smoothatt;
}

float3 ComputePointLight(float3 baseColor, LIGHT l, MATERIAL mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = l.gPosition - pos;
    float d = length(lightVec);
    
    if (d > l.gFalloffEnd)
    {
        return 0.0f;
    }

    lightVec /= d;
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStr = l.gAlbedo * l.gTempIntensity * ndotl;
    float att = CalcAttenuation(d, l.gFalloffStart, l.gFalloffEnd);
    lightStr *= att;

    return Disney_BRDF(lightVec, toEye, normal, lightStr, baseColor, mat);
}

float3 ComputeSpotLight(float3 baseColor, LIGHT l, MATERIAL mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = l.gPosition - pos;
    float d = length(lightVec);

    if (d > l.gFalloffEnd)
    {
        return 0.0f;
    }

    lightVec /= d;
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStr = l.gAlbedo * l.gTempIntensity * ndotl;
    float att = CalcAttenuation(d, l.gFalloffStart, l.gFalloffEnd);
    lightStr *= att;
    float spotFactor = pow(max(dot(-lightVec, l.gDirection), 0.0f), l.gSpotPower);
    lightStr *= spotFactor;

    return Disney_BRDF(lightVec, toEye, normal, lightStr, baseColor, mat);
}
