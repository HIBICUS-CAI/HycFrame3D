struct BASIC_INPUT
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float3 TangentL : TANGENT;
    float2 TexCoordL : TEXCOORD;
    float4 Weight : BLENDWEIGHT;
    uint4 BoneID : BLENDINDICES;
};

float4 main(BASIC_INPUT input) : SV_Position
{
    return float4(1, 1, 1, 1);
}
