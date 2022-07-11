struct VS_INPUT
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float3 TangentL : TANGENT;
    float2 TexCoordL : TEXCOORD;
};

struct VS_OUTPUT
{
    float3 CenterW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float4 TexCoordL : TEXCOORD;
    float2 SizeW : SIZE;
    bool IsBillboard : FLAG;
};

struct MATERIAL
{
    uint gMajorIndex;
    uint gMinorIndex;
    float gFactor;
};

struct INSTANCE_DATA
{
    matrix gWorld;
    MATERIAL gMaterial;
    float4 gCustomizedData1;
    float4 gCustomizedData2;
};

StructuredBuffer<INSTANCE_DATA> gInstances : register(t0);

VS_OUTPUT main(VS_INPUT _in, uint _instanceID : SV_InstanceID)
{
    VS_OUTPUT _out = (VS_OUTPUT)0;

    float4 posH = float4(_in.PosL, 1.0f);
    _out.NormalW = _in.NormalL;
    _out.TangentW = _in.TangentL;

    _out.CenterW = mul(posH, gInstances[_instanceID].gWorld).xyz;
    _out.NormalW = mul(_out.NormalW, (float3x3)gInstances[_instanceID].gWorld);
    _out.TangentW = mul(_out.TangentW, (float3x3)gInstances[_instanceID].gWorld);
    _out.TexCoordL = gInstances[_instanceID].gCustomizedData2;
    _out.SizeW = gInstances[_instanceID].gCustomizedData1.xy;
    if (gInstances[_instanceID].gCustomizedData1.z > 0.0f)
    {
        _out.IsBillboard = true;
    }
    else
    {
        _out.IsBillboard = false;
    }

    return _out;
}
