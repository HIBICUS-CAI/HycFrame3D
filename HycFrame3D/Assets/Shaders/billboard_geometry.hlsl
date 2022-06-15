struct VS_OUTPUT
{
    float3 CenterW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float4 TexCoordL : TEXCOORD;
    float2 SizeW : SIZE;
    bool IsBillboard : FLAG;
};

struct GS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexCoordL : TEXCOORD;
}
