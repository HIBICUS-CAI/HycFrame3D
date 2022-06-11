struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float4 DiffuseAlbedo : COLOR0;
    float4 FresnelShiniese : COLOR1;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexCoordL : TEXCOORD;
    uint UseBumped : BLENDINDICES;
};

struct PS_OUTPUT
{
    float4 Diffuse : SV_TARGET0;
    uint4 Normal : SV_TARGET1;
    float4 WorldPos : SV_TARGET2;
    float4 DiffAlbe : SV_TARGET3;
    float4 FresShin : SV_TARGET4;
};

Texture2D gDiffuse : register(t0);
Texture2D gBumped : register(t1);

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

uint FloatToUint8(float v)
{
    return round((v + 1.f) / 2.f * 65535.f);
}

float Uint8ToFloat(uint v)
{
    return float(v) / 65535.f * 2.f - 1.f;
}

uint3 FloatToUint8_V(float3 v)
{
    uint3 res;
    res.x = FloatToUint8(v.x);
    res.y = FloatToUint8(v.y);
    res.z = FloatToUint8(v.z);
    return res;
}

float3 Uint8ToFloat_V(uint3 v)
{
    float3 res;
    res.x = Uint8ToFloat(v.x);
    res.y = Uint8ToFloat(v.y);
    res.z = Uint8ToFloat(v.z);
    return res;
}

uint PackUint8To16(uint v1, uint v2)
{
    uint final = (v1 << 8) | (v2 & 0xff);
    return final;
}

uint2 UnpackUint16To8(uint v)
{
    uint2 final;
    final.x = v >> 8;
    final.y = v & 0xff;
    return final;
}

uint3 TempUnpack(uint3 packed)
{
    packed.xy = UnpackUint16To8(packed.x);
    return packed;
}

PS_OUTPUT main(VS_OUTPUT _input)
{
    float3 unitNormal = _input.NormalW;
    
    if (_input.UseBumped == 1)
    {
        float3 noramlSample = gBumped.Sample(gLinearSampler, _input.TexCoordL).rgb;
        _input.NormalW = ClacBumpedNormal(noramlSample, unitNormal, _input.TangentW);
    }

    _input.NormalW = normalize(_input.NormalW);
    uint3 norU = FloatToUint8_V(_input.NormalW);
    // norU.x = PackUint8To16(norU.x, norU.y);
    // norU.y = 0;

    PS_OUTPUT _out = (PS_OUTPUT)0;
    _out.WorldPos = float4(_input.PosW, 0.0f);
    _out.Normal = uint4(norU, 0);
    _out.Diffuse = gDiffuse.Sample(gLinearSampler,_input.TexCoordL);
    _out.DiffAlbe = _input.DiffuseAlbedo;
    _out.FresShin = _input.FresnelShiniese;
    
    return _out;
}
