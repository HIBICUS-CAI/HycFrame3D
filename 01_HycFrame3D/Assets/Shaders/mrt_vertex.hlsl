struct VS_INPUT
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float3 TangentL : TANGENT;
    float2 TexCoordL : TEXCOORD;
#if defined (ANIMATION_VERTEX)
    float4 Weight : BLENDWEIGHT;
    uint4 BoneID : BLENDINDICES;
#endif
};

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

struct VIEWPROJ
{
    matrix gView;
    matrix gProjection;
};

struct MATERIAL_INPUT
{
    uint gMajorIndex;
    uint gMinorIndex;
    float gFactor;
};

struct INSTANCE_DATA
{
    matrix gWorld;
    MATERIAL_INPUT gMaterial;
    float4 gCustomizedData1;
    float4 gCustomizedData2;
};

StructuredBuffer<VIEWPROJ> gViewProj : register(t0);
StructuredBuffer<INSTANCE_DATA> gInstances : register(t1);
#if defined (ANIMATION_VERTEX)
StructuredBuffer<matrix> gBoneTransforms : register(t2);
#endif

VS_OUTPUT main(VS_INPUT _in, uint _instanceID : SV_InstanceID)
{
    VS_OUTPUT _out = (VS_OUTPUT)0;

    _out.PosH = float4(_in.PosL, 1.0f);
    _out.NormalW = _in.NormalL;
    _out.TangentW = _in.TangentL;

#if defined (ANIMATION_VERTEX)
    float3 posL = float3(0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
    float3 tangentL = float3(0.0f, 0.0f, 0.0f);

    uint boneID0 = _in.BoneID.x + _instanceID * 256;
    uint boneID1 = _in.BoneID.y + _instanceID * 256;
    uint boneID2 = _in.BoneID.z + _instanceID * 256;
    uint boneID3 = _in.BoneID.w + _instanceID * 256;
    float weight0 = _in.Weight.x;
    float weight1 = _in.Weight.y;
    float weight2 = _in.Weight.z;
    float weight3 = _in.Weight.w;

    posL += weight0 * mul(_out.PosH, gBoneTransforms[boneID0]).xyz;
    posL += weight1 * mul(_out.PosH, gBoneTransforms[boneID1]).xyz;
    posL += weight2 * mul(_out.PosH, gBoneTransforms[boneID2]).xyz;
    posL += weight3 * mul(_out.PosH, gBoneTransforms[boneID3]).xyz;
    _out.PosH = float4(posL, 1.0f);

    normalL += weight0 * mul(_out.NormalW, (float3x3)gBoneTransforms[boneID0]);
    normalL += weight1 * mul(_out.NormalW, (float3x3)gBoneTransforms[boneID1]);
    normalL += weight2 * mul(_out.NormalW, (float3x3)gBoneTransforms[boneID2]);
    normalL += weight3 * mul(_out.NormalW, (float3x3)gBoneTransforms[boneID3]);
    _out.NormalW = normalL;

    tangentL += weight0 * mul(_out.TangentW, (float3x3)gBoneTransforms[boneID0]);
    tangentL += weight1 * mul(_out.TangentW, (float3x3)gBoneTransforms[boneID1]);
    tangentL += weight2 * mul(_out.TangentW, (float3x3)gBoneTransforms[boneID2]);
    tangentL += weight3 * mul(_out.TangentW, (float3x3)gBoneTransforms[boneID3]);
    _out.TangentW = tangentL;
#endif

    _out.PosH = mul(_out.PosH, gInstances[_instanceID].gWorld);
    _out.PosW = _out.PosH.xyz;
    _out.DiffuseAlbedo = float4(0.8f, 0.8f, 0.8f, 1.f);
    _out.MaterialIndexFactor = uint3(
        gInstances[_instanceID].gMaterial.gMajorIndex,
        gInstances[_instanceID].gMaterial.gMinorIndex,
        asuint(gInstances[_instanceID].gMaterial.gFactor));
    _out.NormalW = mul(_out.NormalW, (float3x3)gInstances[_instanceID].gWorld);
    _out.TangentW = mul(_out.TangentW, (float3x3)gInstances[_instanceID].gWorld);
    _out.PosH = mul(_out.PosH, gViewProj[0].gView);
    _out.PosH = mul(_out.PosH, gViewProj[0].gProjection);
    _out.TexCoordL = _in.TexCoordL;
    if (gInstances[_instanceID].gCustomizedData1.x > 0.0f)
    {
        _out.UsePBRTex.x = 1;
    }
    else
    {
        _out.UsePBRTex.x = 0;
    }
    if (gInstances[_instanceID].gCustomizedData1.y > 0.0f)
    {
        _out.UsePBRTex.y = 1;
    }
    else
    {
        _out.UsePBRTex.y = 0;
    }
    if (gInstances[_instanceID].gCustomizedData1.z > 0.0f)
    {
        _out.UsePBRTex.z = 1;
    }
    else
    {
        _out.UsePBRTex.z = 0;
    }
    if (gInstances[_instanceID].gCustomizedData1.w > 0.0f)
    {
        _out.UsePBRTex.w = 1;
        _out.EmissiveIntensity = gInstances[_instanceID].gCustomizedData2.x;
    }
    else
    {
        _out.UsePBRTex.w = 0;
    }

    return _out;
}
