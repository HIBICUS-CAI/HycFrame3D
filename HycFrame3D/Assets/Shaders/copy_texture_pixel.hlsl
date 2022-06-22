struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoordL : TEXCOORD;
};

SamplerState gSamLinearWrap : register(s0);

Texture2D gSourceTexture : register(t0);

float4 main(VS_OUTPUT _in) : SV_TARGET
{
    return gSourceTexture.Sample(gSamLinearWrap, _in.TexCoordL);
}
