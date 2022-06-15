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
};

struct VIEWPROJ_UPVEC_POS
{
    matrix gView;
    matrix gProjection;
    float3 gCamUpVec;
    float3 gCamPos;
};

StructuredBuffer<VIEWPROJ_UPVEC_POS> gViewProjCamUp : register(t0);

[maxvertexcount(4)]
void main(point VS_OUTPUT _in[1], inout TriangleStream<GS_OUTPUT> _triStream)
{
    float3 up, look, right;

    if (_in[0].IsBillboard)
    {
        look = gViewProjCamUp[0].gCamPos - _in[0].CenterW;
        look = normalize(look);
        up = normalize(normalize(gViewProjCamUp[0].gCamUpVec) -
            (dot(normalize(gViewProjCamUp[0].gCamUpVec), look * -1.f) * look * -1.f));
        right = cross(look, up);
    }
    else
    {
        look = normalize(_in[0].NormalW);
        right = normalize(_in[0].TangentW);
        up = cross(right, look);
    }

    float hWidth = 0.5f * _in[0].SizeW.x;
    float hHeight = 0.5f * _in[0].SizeW.y;

    float4 vertex[4];
    vertex[0] = float4(_in[0].CenterW - hWidth * right - hHeight * up, 1.f);
    vertex[1] = float4(_in[0].CenterW - hWidth * right + hHeight * up, 1.f);
    vertex[2] = float4(_in[0].CenterW + hWidth * right - hHeight * up, 1.f);
    vertex[3] = float4(_in[0].CenterW + hWidth * right + hHeight * up, 1.f);

    float2 texcd[4];
    {
        float4 texInfo = _in[0].TexCoordL;
        texcd[0] = float2(texInfo.x, texInfo.y + texInfo.w);
        texcd[1] = float2(texInfo.x, texInfo.y);
        texcd[2] = float2(texInfo.x + texInfo.z, texInfo.y + texInfo.w);
        texcd[3] = float2(texInfo.x + texInfo.z, texInfo.y);
    }

    GS_OUTPUT gout;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(vertex[i], gViewProjCamUp[0].gView);
        gout.PosH = mul(gout.PosH, gViewProjCamUp[0].gProjection);
        gout.PosW = vertex[i].xyz;
        gout.NormalW = look;
        gout.TangentW = right;
        gout.TexCoordL = texcd[i];

        _triStream.Append(gout);
    }
}
