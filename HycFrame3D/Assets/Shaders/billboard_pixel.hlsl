struct GS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexCoordL : TEXCOORD;
};

Texture2D gDiffuse : register(t0);
SamplerState gSamLinear : register(s0);

float4 main(GS_OUTPUT _in) : SV_TARGET
{
    float4 color = gDiffuse.Sample(gSamLinear, _in.TexCoordL);
    clip(color.a - 0.1f);
    return color;
}
