uint FloatToUint8_S(float _v)
{
    return round((_v + 1.f) / 2.f * 255.f);
}

float Uint8ToFloat_S(uint _v)
{
    return float(_v) / 255.f * 2.f - 1.f;
}

uint FloatToUint16_S(float _v)
{
    return round((_v + 1.f) / 2.f * 65535.f);
}

float Uint16ToFloat_S(uint _v)
{
    return float(_v) / 65535.f * 2.f - 1.f;
}

uint4 FloatToUint8_V4(float4 _v)
{
    uint4 res;
    res.x = FloatToUint8_S(_v.x);
    res.y = FloatToUint8_S(_v.y);
    res.z = FloatToUint8_S(_v.z);
    res.w = FloatToUint8_S(_v.w);
    return res;
}

float4 Uint8ToFloat_V4(uint4 _v)
{
    float4 res;
    res.x = Uint8ToFloat_S(_v.x);
    res.y = Uint8ToFloat_S(_v.y);
    res.z = Uint8ToFloat_S(_v.z);
    res.w = Uint8ToFloat_S(_v.w);
    return res;
}

uint2 FloatToUint16_V2(float2 _v)
{
    uint2 res;
    res.x = FloatToUint8_S(_v.x);
    res.y = FloatToUint8_S(_v.y);
    return res;
}

float2 Uint16ToFloat_V2(uint2 _v)
{
    float2 res;
    res.x = Uint8ToFloat_S(_v.x);
    res.y = Uint8ToFloat_S(_v.y);
    return res;
}

uint PackFourUint8ToUint32(uint _v0, uint _v1, uint _v2, uint _v3)
{
    uint v16Left = (_v0 << 8) | (_v1 & 0xff);
    uint v16Right = (_v2 << 8) | (_v3 & 0xff);
    uint res = (v16Left << 16) | (v16Right & 0xffff);
    return res;
}

uint4 UnpackUint32ToFourUint8(uint _v)
{
    uint2 temp;
    temp.x = _v >> 16;
    temp.y = _v & 0xffff;
    uint4 res;
    res.x = temp.x >> 8;
    res.y = temp.x & 0xff;
    res.z = temp.y >> 8;
    res.w = temp.y & 0xff;
    return res;
}

uint PackTwoUint16ToUint32(uint _v0, uint _v1)
{
    uint res = (_v0 << 16) | (_v1 & 0xffff);
    return res;
}

uint2 UnpackUint32ToTwoUint16(uint _v)
{
    uint2 res;
    res.x = _v >> 16;
    res.y = _v & 0xffff;
    return res;
}

float2 EncodeNormalizeVec(float3 _v)
{
    return (float2(atan2(_v.y, _v.x) / 3.1415926536f, _v.z) + 1.f) * 0.5f;
}

float3 DecodeNormalizeVec(float2 _v)
{
    float2 ang = _v * 2.f - 1.f;
    float2 scth;
    sincos(ang.x * 3.1415926536f, scth.x, scth.y);
    float2 scphi = float2(sqrt(1.f - ang.y * ang.y), ang.y);
    return float3(scth.y * scphi.x, scth.x * scphi.x, scphi.y);
}

float3 GetNormalFromGeoValue(uint _v)
{
    float2 unpack = Uint16ToFloat_V2(UnpackUint32ToTwoUint16(_v));
    return DecodeNormalizeVec(unpack);
}