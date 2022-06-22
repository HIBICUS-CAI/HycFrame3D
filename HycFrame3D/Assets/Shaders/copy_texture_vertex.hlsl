struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoordL : TEXCOORD;
};

static const float2 gTexCoords[4] = 
{
    float2(0.0f, 1.0f), float2(0.0f, 0.0f),
    float2(1.0f, 0.0f), float2(1.0f, 1.0f)
};

static const float4 gPosH[4] = 
{
    float4(-1.0f, -1.0f, 0.0f, 1.0f), float4(-1.0f, 1.0f, 0.0f, 1.0f),
    float4(1.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, -1.0f, 0.0f, 1.0f)
};

VS_OUTPUT main(uint _vid : SV_VertexID)
{
    VS_OUTPUT output = (VS_OUTPUT)0.0f;
    
    output.TexCoordL = gTexCoords[_vid];
    output.PosH = gPosH[_vid];

    return output;
}
