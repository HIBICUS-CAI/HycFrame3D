struct VS_INPUT
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float3 TangentL : TANGENT;
    float2 TexCoordL : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float4 ColorOffset : COLOR;
    float2 TexCoordL : TEXCOORD;
};

struct PROJ
{
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

StructuredBuffer<PROJ> gProj : register(t0);
StructuredBuffer<INSTANCE_DATA> gInstances : register(t1);

VS_OUTPUT main(VS_INPUT _in, uint _instanceID : SV_InstanceID, uint _vertexId : SV_VertexID)
{
    VS_OUTPUT _out = (VS_OUTPUT)0;

    _out.PosH = mul(float4(_in.PosL, 1.0f), gInstances[_instanceID].gWorld);
    _out.PosH = mul(_out.PosH, gProj[0].gProjection);
    _out.PosH.z = 0.0f;
    _out.ColorOffset = gInstances[_instanceID].gCustomizedData1;

    const uint vertexToUV[4]= { 1, 0, 2, 3 };

    float uv[2][2] =
    {
        {gInstances[_instanceID].gCustomizedData2.x, gInstances[_instanceID].gCustomizedData2.z},
        {gInstances[_instanceID].gCustomizedData2.y, gInstances[_instanceID].gCustomizedData2.w}
    };

    uint uIndex = (vertexToUV[_vertexId] & 2) >> 1;
    uint vIndex = (vertexToUV[_vertexId] & 1);

    _out.TexCoordL = float2(uv[0][uIndex], uv[1][vIndex]);

    return _out;
}
