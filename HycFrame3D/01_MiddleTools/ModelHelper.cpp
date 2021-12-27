#define _CRT_SECURE_NO_WARNINGS

#include "ModelHelper.h"
#include "PrintLog.h"
#include <fstream>
#include <cstdio>
#include <cstring>
#include "rapidjson\filereadstream.h"
#include "rapidjson\writer.h"
#include "rapidjson\document.h"
#include "DDSTextureLoader11.h"
#include "WICTextureLoader11.h"
#include "RSRoot_DX11.h"
#include "RSMeshHelper.h"
#include "RSDevices.h"
#include "RSResourceManager.h"

void LoadByBinary(const std::string _filePath, RS_SUBMESH_DATA* _result);
void LoadByJson(const std::string _filePath, RS_SUBMESH_DATA* _result);

void LoadModelFile(const std::string _filePath, MODEL_FILE_TYPE _type,
    RS_SUBMESH_DATA* _result)
{
    switch (_type)
    {
    case MODEL_FILE_TYPE::BIN:
        LoadByBinary(_filePath, _result); break;
    case MODEL_FILE_TYPE::JSON:
        LoadByJson(_filePath, _result); break;
    default:
        break;
    }
}

void LoadByBinary(const std::string _filePath, RS_SUBMESH_DATA* _result)
{

}

void LoadByJson(const std::string _filePath, RS_SUBMESH_DATA* _result)
{
    std::FILE* fp = std::fopen(_filePath.c_str(), "rb");
    if (!fp) { P_LOG(LOG_ERROR, "cannt open file : %s\n", _filePath.c_str()); }
    char* readBuf = new char[65536];
#ifdef _DEBUG
    assert(readBuf);
#endif // _DEBUG

    rapidjson::FileReadStream is(fp, readBuf, 65536 * sizeof(char));
    rapidjson::Document doc = {};
    doc.ParseStream(is);
    if (doc.HasParseError())
    {
        P_LOG(LOG_ERROR, "json error code : %d\n", doc.GetParseError());
#ifdef _DEBUG
        bool json_file_invalid = false;
        assert(json_file_invalid);
#endif // _DEBUG
    }
    delete[] readBuf;
    std::fclose(fp);

    std::string directory = doc["directory"].GetString();
    std::string texType = doc["texture-type"].GetString();

    UINT subSize = doc["sub-model-size"].GetUint();
#ifdef _DEBUG
    assert(subSize == 1);
#endif // _DEBUG
    UINT subArraySize = doc["sub-model"].Size();
#ifdef _DEBUG
    assert(subSize == subArraySize);
#endif // _DEBUG

    std::vector<UINT> index = {};
    std::vector<VertexType::TangentVertex> vertex = {};
    std::vector<MODEL_TEXTURE_INFO> texture = {};
    for (UINT i = 0; i < subSize; i++)
    {
        index.clear();
        vertex.clear();
        texture.clear();
        UINT indexSize = doc["sub-model"][i]["index-size"].GetUint();
#ifdef _DEBUG
        assert(indexSize == (UINT)doc["sub-model"][i]["index"].Size());
#endif // _DEBUG
        UINT vertexSize = doc["sub-model"][i]["vertex-size"].GetUint();
#ifdef _DEBUG
        assert(vertexSize == (UINT)doc["sub-model"][i]["vertex"].Size());
#endif // _DEBUG
        UINT texSize = doc["sub-model"][i]["texture-size"].GetUint();
#ifdef _DEBUG
        assert(texSize == (UINT)doc["sub-model"][i]["texture"].Size());
#endif // _DEBUG

        for (UINT j = 0; j < indexSize; j++)
        {
            index.push_back(doc["sub-model"][i]["index"][j].GetUint());
        }

        for (UINT j = 0; j < vertexSize; j++)
        {
            static VertexType::TangentVertex v = {};
            v = {};

            v.Position.x =
                doc["sub-model"][i]["vertex"][j]["position"][0].GetFloat();
            v.Position.y =
                doc["sub-model"][i]["vertex"][j]["position"][1].GetFloat();
            v.Position.z =
                doc["sub-model"][i]["vertex"][j]["position"][2].GetFloat();

            v.Normal.x =
                doc["sub-model"][i]["vertex"][j]["normal"][0].GetFloat();
            v.Normal.y =
                doc["sub-model"][i]["vertex"][j]["normal"][1].GetFloat();
            v.Normal.z =
                doc["sub-model"][i]["vertex"][j]["normal"][2].GetFloat();

            v.Tangent.x =
                doc["sub-model"][i]["vertex"][j]["tangent"][0].GetFloat();
            v.Tangent.y =
                doc["sub-model"][i]["vertex"][j]["tangent"][1].GetFloat();
            v.Tangent.z =
                doc["sub-model"][i]["vertex"][j]["tangent"][2].GetFloat();

            v.TexCoord.x =
                doc["sub-model"][i]["vertex"][j]["texcoord"][0].GetFloat();
            v.TexCoord.y =
                doc["sub-model"][i]["vertex"][j]["texcoord"][1].GetFloat();

            vertex.push_back(v);
        }

        for (UINT j = 0; j < texSize; j++)
        {
            static MODEL_TEXTURE_INFO t = {};
            t = {};
            t.mType = doc["sub-model"][i]["texture"][j]["type"].GetString();
            t.mPath = doc["sub-model"][i]["texture"][j]["path"].GetString();

            texture.push_back(t);
        }

        SUBMESH_INFO si = {};
        si.mTopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
        si.mIndeices = &index;
        si.mVerteices = &vertex;
        std::vector<std::string> t = {};
        for (auto& tex : texture) { t.emplace_back(tex.mPath); }
        si.mTextures = &t;
        si.mStaticMaterial = "copper";
        GetRSRoot_DX11_Singleton()->MeshHelper()->ProcessSubMesh(_result,
            &si, LAYOUT_TYPE::NORMAL_TANGENT_TEX);
    }
}

void AddBumpedTexTo(RS_SUBMESH_DATA* _result, std::string _filePath)
{
    RSRoot_DX11* root = GetRSRoot_DX11_Singleton();

    static std::wstring wstr = L"";
    static std::string name = "";
    static HRESULT hr = S_OK;
    ID3D11ShaderResourceView* srv = nullptr;

    wstr = std::wstring(_filePath.begin(), _filePath.end());
    wstr = L".\\Assets\\Textures\\" + wstr;

    if (_filePath.find(".dds") != std::string::npos ||
        _filePath.find(".DDS") != std::string::npos)
    {
        hr = DirectX::CreateDDSTextureFromFile(root->Devices()->GetDevice(),
            wstr.c_str(), nullptr, &srv);
        if (SUCCEEDED(hr))
        {
            name = _filePath;
            root->ResourceManager()->AddMeshSrv(name, srv);
        }
        else
        {
            bool texture_load_fail = false;
            assert(texture_load_fail);
        }
    }
    else
    {
        hr = DirectX::CreateWICTextureFromFile(root->Devices()->GetDevice(),
            wstr.c_str(), nullptr, &srv);
        if (SUCCEEDED(hr))
        {
            name = _filePath;
            root->ResourceManager()->AddMeshSrv(name, srv);
        }
        else
        {
            bool texture_load_fail = false;
            assert(texture_load_fail);
        }
    }

    _result->mTextures.emplace_back(name);
}
