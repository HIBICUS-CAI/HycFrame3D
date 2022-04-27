struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoordL : TEXCOORD;
};

SamplerState gSamLinearWrap : register(s0);

Texture2D gDiffuse : register(t0);
Texture2D gDiffuseAlbedo : register(t1);
Texture2D gSsao : register(t2);

float4 main(VS_OUTPUT _in) : SV_TARGET
{
    float4 diffuse = gDiffuse.Sample(gSamLinearWrap, _in.TexCoordL);
    float4 albedo = gDiffuseAlbedo.Sample(gSamLinearWrap, _in.TexCoordL);
    float access = gSsao.SampleLevel(gSamLinearWrap, _in.TexCoordL, 0.0f).r;
    access = access * access;

    float4 final = access * albedo;
    final *= diffuse;
    final.a = diffuse.a;

    return final;
}
