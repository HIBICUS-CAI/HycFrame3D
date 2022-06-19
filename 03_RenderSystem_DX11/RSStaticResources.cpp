//---------------------------------------------------------------
// File: RSStaticResources.cpp
// Proj: RenderSystem_DX11
// Info: 保存并提供所有常用的资源及其引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSStaticResources.h"
#include "RSRoot_DX11.h"
#include "RSPipeline.h"
#include "RSTopic.h"
#include "RSShaderCompile.h"
#include "RSUtilityFunctions.h"
#include "RSDevices.h"

RSStaticResources::RSStaticResources() :
    mRootPtr(nullptr),
    mVertexShaderMap({}), mGeometryShaderMap({}),
    mPixelShaderMap({}), mComputeShaderMap({}),
    mRasterizerStateMap({}), mDepthStencilStateMap({}),
    mBlendStateMap({}), mSamplerMap({}), mInputLayoutMap({}),
    mStaticPipelineMap({}), mStaticTopicMap({}),
    mMaterialVector({}), mMaterialIndexMap({}),
    mMaterialBuffer(nullptr), mMaterialBufferSrv(nullptr)
{

}

RSStaticResources::~RSStaticResources()
{

}

bool RSStaticResources::StartUp(RSRoot_DX11* _root)
{
    if (!_root) { return false; }

    mRootPtr = _root;

    if (!CompileStaticShaders()) { return false; }
    if (!BuildStaticStates()) { return false; }
    if (!BuildStaticInputLayouts()) { return false; }
    if (!BuildStaticTopics()) { return false; }
    if (!BuildStaticPipelines()) { return false; }
    if (!BuildStaticMaterials()) { return false; }

    return true;
}

void RSStaticResources::CleanAndStop()
{
    for (auto& vShader : mVertexShaderMap)
    {
        SAFE_RELEASE(vShader.second);
    }
    mVertexShaderMap.clear();

    for (auto& gShader : mGeometryShaderMap)
    {
        SAFE_RELEASE(gShader.second);
    }
    mGeometryShaderMap.clear();

    for (auto& pShader : mPixelShaderMap)
    {
        SAFE_RELEASE(pShader.second);
    }
    mPixelShaderMap.clear();

    for (auto& cShader : mComputeShaderMap)
    {
        SAFE_RELEASE(cShader.second);
    }
    mComputeShaderMap.clear();

    for (auto& rState : mRasterizerStateMap)
    {
        SAFE_RELEASE(rState.second);
    }
    mRasterizerStateMap.clear();

    for (auto& dState : mDepthStencilStateMap)
    {
        SAFE_RELEASE(dState.second);
    }
    mDepthStencilStateMap.clear();

    for (auto& bState : mBlendStateMap)
    {
        SAFE_RELEASE(bState.second);
    }
    mBlendStateMap.clear();

    for (auto& sState : mSamplerMap)
    {
        SAFE_RELEASE(sState.second);
    }
    mSamplerMap.clear();

    for (auto& layout : mInputLayoutMap)
    {
        SAFE_RELEASE(layout.second);
    }
    mInputLayoutMap.clear();

    for (auto& pipeline : mStaticPipelineMap)
    {
        pipeline.second->ReleasePipeline();
        delete pipeline.second;
        pipeline.second = nullptr;
    }
    mStaticPipelineMap.clear();

    for (auto& topic : mStaticTopicMap)
    {
        topic.second->ReleaseTopic();
        delete topic.second;
        topic.second = nullptr;
    }
    mStaticTopicMap.clear();

    mMaterialVector.clear();
    mMaterialIndexMap.clear();
    SAFE_RELEASE(mMaterialBufferSrv);
    SAFE_RELEASE(mMaterialBuffer)
}

bool RSStaticResources::CompileStaticShaders()
{
    // TEMP----------------------
    return true;
    // TEMP----------------------
}

bool RSStaticResources::BuildStaticStates()
{
    // TEMP----------------------
    return true;
    // TEMP----------------------
}

bool RSStaticResources::BuildStaticInputLayouts()
{
    ID3DBlob* shaderBlob = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;
    HRESULT hr = S_OK;

    {
        hr = Tool::CompileShaderFromFile(
            L"RenderSystem_StaticResources\\InputLayouts\\input_layout_basic.hlsl",
            "main", "vs_5_0", &shaderBlob);
        FAIL_HR_RETURN(hr);

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            {
                "POSITION",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,0,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "NORMAL",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,12,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "TEXCOORD",0,
                DXGI_FORMAT_R32G32_FLOAT,0,24,
                D3D11_INPUT_PER_VERTEX_DATA,0
            }
        };
        UINT numInputLayouts = ARRAYSIZE(layout);

        hr = mRootPtr->Devices()->GetDevice()->
            CreateInputLayout(
                layout, numInputLayouts,
                shaderBlob->GetBufferPointer(),
                shaderBlob->GetBufferSize(),
                &inputLayout);
        shaderBlob->Release();
        FAIL_HR_RETURN(hr);
        mInputLayoutMap.insert({ "BasicVertex",inputLayout });
        shaderBlob = nullptr;
        inputLayout = nullptr;
        hr = S_OK;
    }

    {
        hr = Tool::CompileShaderFromFile(
            L"RenderSystem_StaticResources\\InputLayouts\\input_layout_color.hlsl",
            "main", "vs_5_0", &shaderBlob);
        FAIL_HR_RETURN(hr);

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            {
                "POSITION",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,0,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "NORMAL",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,12,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "COLOR",0,
                DXGI_FORMAT_R32G32B32A32_FLOAT,0,24,
                D3D11_INPUT_PER_VERTEX_DATA,0
            }
        };
        UINT numInputLayouts = ARRAYSIZE(layout);

        hr = mRootPtr->Devices()->GetDevice()->
            CreateInputLayout(
                layout, numInputLayouts,
                shaderBlob->GetBufferPointer(),
                shaderBlob->GetBufferSize(),
                &inputLayout);
        shaderBlob->Release();
        FAIL_HR_RETURN(hr);
        mInputLayoutMap.insert({ "ColorVertex",inputLayout });
        shaderBlob = nullptr;
        inputLayout = nullptr;
        hr = S_OK;
    }

    {
        hr = Tool::CompileShaderFromFile(
            L"RenderSystem_StaticResources\\InputLayouts\\input_layout_tangent.hlsl",
            "main", "vs_5_0", &shaderBlob);
        FAIL_HR_RETURN(hr);

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            {
                "POSITION",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,0,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "NORMAL",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,12,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "TANGENT",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,24,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "TEXCOORD",0,
                DXGI_FORMAT_R32G32_FLOAT,0,36,
                D3D11_INPUT_PER_VERTEX_DATA,0
            }
        };
        UINT numInputLayouts = ARRAYSIZE(layout);

        hr = mRootPtr->Devices()->GetDevice()->
            CreateInputLayout(
                layout, numInputLayouts,
                shaderBlob->GetBufferPointer(),
                shaderBlob->GetBufferSize(),
                &inputLayout);
        shaderBlob->Release();
        FAIL_HR_RETURN(hr);
        mInputLayoutMap.insert({ "TangentVertex",inputLayout });
        shaderBlob = nullptr;
        inputLayout = nullptr;
        hr = S_OK;
    }

    {
        hr = Tool::CompileShaderFromFile(
            L"RenderSystem_StaticResources\\InputLayouts\\input_layout_animation.hlsl",
            "main", "vs_5_0", &shaderBlob);
        FAIL_HR_RETURN(hr);

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            {
                "POSITION",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,0,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "NORMAL",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,12,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "TANGENT",0,
                DXGI_FORMAT_R32G32B32_FLOAT,0,24,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "TEXCOORD",0,
                DXGI_FORMAT_R32G32_FLOAT,0,36,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "BLENDWEIGHT",0,
                DXGI_FORMAT_R32G32B32A32_FLOAT,0,44,
                D3D11_INPUT_PER_VERTEX_DATA,0
            },
            {
                "BLENDINDICES",0,
                DXGI_FORMAT_R32G32B32A32_UINT,0,60,
                D3D11_INPUT_PER_VERTEX_DATA,0
            }
        };
        UINT numInputLayouts = ARRAYSIZE(layout);

        hr = mRootPtr->Devices()->GetDevice()->
            CreateInputLayout(
                layout, numInputLayouts,
                shaderBlob->GetBufferPointer(),
                shaderBlob->GetBufferSize(),
                &inputLayout);
        shaderBlob->Release();
        FAIL_HR_RETURN(hr);
        mInputLayoutMap.insert({ "AnimationVertex",inputLayout });
        shaderBlob = nullptr;
        inputLayout = nullptr;
        hr = S_OK;
    }

    return true;
}

bool RSStaticResources::BuildStaticTopics()
{
    // TEMP----------------------
    return true;
    // TEMP----------------------
}

bool RSStaticResources::BuildStaticPipelines()
{
    // TEMP----------------------
    return true;
    // TEMP----------------------
}

bool RSStaticResources::BuildStaticMaterials()
{
    Tool::Json::JsonFile matFile = {};
    Tool::Json::LoadJsonFile(&matFile,
        "RenderSystem_StaticResources\\Materials\\static-materials.json");
    if (matFile.HasParseError()) { return false; }

    UINT matSize = matFile["static-material"].Size();
    mMaterialVector.resize(matSize);
    std::string matName = "";

    for (UINT i = 0; i < matSize; i++)
    {
        mMaterialVector[i].mFresnelR0.x =
            matFile["static-material"][i]["fresnel-r0"][0].GetFloat();
        mMaterialVector[i].mFresnelR0.y =
            matFile["static-material"][i]["fresnel-r0"][1].GetFloat();
        mMaterialVector[i].mFresnelR0.z =
            matFile["static-material"][i]["fresnel-r0"][2].GetFloat();
        mMaterialVector[i].mSubSurface =
            matFile["static-material"][i]["subsurface"].GetFloat();
        mMaterialVector[i].mMetallic =
            matFile["static-material"][i]["metallic"].GetFloat();
        mMaterialVector[i].mSpecular =
            matFile["static-material"][i]["specular"].GetFloat();
        mMaterialVector[i].mSpecularTint =
            matFile["static-material"][i]["specular-tint"].GetFloat();
        mMaterialVector[i].mRoughness =
            matFile["static-material"][i]["roughness"].GetFloat();
        mMaterialVector[i].mAnisotropic =
            matFile["static-material"][i]["anisotropic"].GetFloat();
        mMaterialVector[i].mSheen =
            matFile["static-material"][i]["sheen"].GetFloat();
        mMaterialVector[i].mSheenTint =
            matFile["static-material"][i]["sheen-tint"].GetFloat();
        mMaterialVector[i].mClearcoat =
            matFile["static-material"][i]["clearcoat"].GetFloat();
        mMaterialVector[i].mClearcoatGloss =
            matFile["static-material"][i]["clearcoat-gloss"].GetFloat();

        matName = matFile["static-material"][i]["name"].GetString();
        mMaterialIndexMap.insert({ matName,i });
    }

    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bufDesc = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    auto devicePtr = mRootPtr->Devices()->GetDevice();
    ZeroMemory(&bufDesc, sizeof(bufDesc));
    ZeroMemory(&srvDesc, sizeof(srvDesc));

    bufDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufDesc.ByteWidth = MAX_STRUCTURED_BUFFER_SIZE * sizeof(RS_MATERIAL_DATA);
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = sizeof(RS_MATERIAL_DATA);
    hr = devicePtr->CreateBuffer(&bufDesc, nullptr, &mMaterialBuffer);
    FAIL_HR_RETURN(hr);

    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = MAX_STRUCTURED_BUFFER_SIZE;
    hr = devicePtr->CreateShaderResourceView(
        mMaterialBuffer, &srvDesc, &mMaterialBufferSrv);
    FAIL_HR_RETURN(hr);

    MapMaterialData();

    return true;
}

ID3D11VertexShader* RSStaticResources::GetStaticVertexShader(
    std::string& _shaderName)
{
    auto found = mVertexShaderMap.find(_shaderName);
    if (found != mVertexShaderMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

ID3D11GeometryShader* RSStaticResources::GetStaticGeometryShader(
    std::string& _shaderName)
{
    auto found = mGeometryShaderMap.find(_shaderName);
    if (found != mGeometryShaderMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

ID3D11PixelShader* RSStaticResources::GetStaticPixelShader(
    std::string& _shaderName)
{
    auto found = mPixelShaderMap.find(_shaderName);
    if (found != mPixelShaderMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

ID3D11ComputeShader* RSStaticResources::GetStaticComputeShader(
    std::string& _shaderName)
{
    auto found = mComputeShaderMap.find(_shaderName);
    if (found != mComputeShaderMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

ID3D11RasterizerState* RSStaticResources::GetStaticRasterizerState(
    std::string& _stateName)
{
    auto found = mRasterizerStateMap.find(_stateName);
    if (found != mRasterizerStateMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

ID3D11DepthStencilState* RSStaticResources::GetStaticDepthStencilState(
    std::string& _stateName)
{
    auto found = mDepthStencilStateMap.find(_stateName);
    if (found != mDepthStencilStateMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

ID3D11BlendState* RSStaticResources::GetStaticBlendState(
    std::string& _stateName)
{
    auto found = mBlendStateMap.find(_stateName);
    if (found != mBlendStateMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

ID3D11SamplerState* RSStaticResources::GetStaticSampler(
    std::string& _samplerName)
{
    auto found = mSamplerMap.find(_samplerName);
    if (found != mSamplerMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

ID3D11InputLayout* RSStaticResources::GetStaticInputLayout(
    std::string& _layoutName)
{
    auto found = mInputLayoutMap.find(_layoutName);
    if (found != mInputLayoutMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

const RSPipeline* const RSStaticResources::GetStaticPipeline(
    std::string& _pipelineName)
{
    auto found = mStaticPipelineMap.find(_pipelineName);
    if (found != mStaticPipelineMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

const RSTopic* const RSStaticResources::GetStaticTopic(
    std::string& _topicName)
{
    auto found = mStaticTopicMap.find(_topicName);
    if (found != mStaticTopicMap.end())
    {
        return found->second;
    }
    else
    {
        return nullptr;
    }
}

UINT RSStaticResources::GetStaticMaterialIndex(std::string& _materialName)
{
    auto found = mMaterialIndexMap.find(_materialName);
    assert(found != mMaterialIndexMap.end());
    return found->second;
}

void RSStaticResources::MapMaterialData()
{
    auto contextPtr = mRootPtr->Devices()->GetSTContext();
    D3D11_MAPPED_SUBRESOURCE msr = {};
    contextPtr->Map(mMaterialBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    RS_MATERIAL_DATA* mat_data = (RS_MATERIAL_DATA*)msr.pData;
    memcpy_s(mat_data,
        mMaterialVector.size() * sizeof(RS_MATERIAL_DATA),
        mMaterialVector.data(),
        mMaterialVector.size() * sizeof(RS_MATERIAL_DATA));
    contextPtr->Unmap(mMaterialBuffer, 0);
}
