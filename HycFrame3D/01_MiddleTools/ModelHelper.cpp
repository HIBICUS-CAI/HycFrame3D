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
    static std::string path = "";
    path = ".\\Assets\\Models\\" + _filePath;

    switch (_type)
    {
    case MODEL_FILE_TYPE::BIN:
        LoadByBinary(path, _result); break;
    case MODEL_FILE_TYPE::JSON:
        LoadByJson(path, _result); break;
    default:
        break;
    }
}

void LoadByBinary(const std::string _filePath, RS_SUBMESH_DATA* _result)
{
    std::ifstream inFile(_filePath, std::ios::in | std::ios::binary);

#ifdef _DEBUG
    assert(inFile.is_open());
#endif // _DEBUG

    std::string directory = "";
    std::string texType = "";
    bool animated = false;

    // directory
    {
        int size = 0;
        char str[128] = "";
        inFile.read((char*)&size, sizeof(size));
        inFile.read(str, size);
        directory = str;
    }

    // texture-type
    {
        int size = 0;
        char str[128] = "";
        inFile.read((char*)&size, sizeof(size));
        inFile.read(str, size);
        texType = str;
    }

    // animation-flag
    {
        int flag = 0;
        inFile.read((char*)&flag, sizeof(flag));
        animated = flag ? true : false;
    }

    // sub-model-size
    int subSize = 0;
    inFile.read((char*)&subSize, sizeof(subSize));
#ifdef _DEBUG
    assert(subSize == 1);
#endif // _DEBUG

    std::vector<UINT> index = {};
    std::vector<VertexType::TangentVertex> vertex = {};
    std::vector<MODEL_TEXTURE_INFO> texture = {};
    int indexSize = 0;
    int vertexSize = 0;
    int textureSize = 0;
    for (int i = 0; i < subSize; i++)
    {
        index.clear();
        vertex.clear();
        texture.clear();

        // each-sub-sizes
        inFile.read((char*)&indexSize, sizeof(indexSize));
        inFile.read((char*)&vertexSize, sizeof(vertexSize));
        inFile.read((char*)&textureSize, sizeof(textureSize));

        // each-sub-index
        UINT ind = 0;
        VertexType::TangentVertex ver = {};
        MODEL_TEXTURE_INFO tex = {};
        for (int j = 0; j < indexSize; j++)
        {
            inFile.read((char*)&ind, sizeof(ind));
            index.push_back(ind);
        }

        // each-sub-vertex
        {
            double temp = 0.0;
            for (int j = 0; j < vertexSize; j++)
            {
                inFile.read((char*)(&temp), sizeof(double));
                ver.Position.x = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                ver.Position.y = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                ver.Position.z = (float)temp;

                inFile.read((char*)(&temp), sizeof(double));
                ver.Normal.x = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                ver.Normal.y = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                ver.Normal.z = (float)temp;

                inFile.read((char*)(&temp), sizeof(double));
                ver.Tangent.x = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                ver.Tangent.y = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                ver.Tangent.z = (float)temp;

                inFile.read((char*)(&temp), sizeof(double));
                ver.TexCoord.x = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                ver.TexCoord.y = (float)temp;

                vertex.push_back(ver);
            }
        }

        // each-sub-texture
        for (int j = 0; j < textureSize; j++)
        {
            int len = 0;
            char str[1024] = "";
            inFile.read((char*)(&len), sizeof(len));
            inFile.read(str, len);
            tex.mType = str;
            std::strcpy(str, "");
            inFile.read((char*)(&len), sizeof(len));
            inFile.read(str, len);
            tex.mPath = str;
            texture.push_back(tex);
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

    inFile.close();
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
    bool animated = doc["with-animation"].GetBool();

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
    std::vector<VertexType::AnimationVertex> aniVertex = {};
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

        if (animated)
        {
            for (UINT j = 0; j < vertexSize; j++)
            {
                static VertexType::AnimationVertex v = {};
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

                v.Weight.x =
                    doc["sub-model"][i]["vertex"][j]["weight"][0].GetFloat();
                v.Weight.y =
                    doc["sub-model"][i]["vertex"][j]["weight"][1].GetFloat();
                v.Weight.z =
                    doc["sub-model"][i]["vertex"][j]["weight"][2].GetFloat();
                v.Weight.w =
                    doc["sub-model"][i]["vertex"][j]["weight"][3].GetFloat();

                v.BoneID.x =
                    doc["sub-model"][i]["vertex"][j]["boneid"][0].GetUint();
                v.BoneID.y =
                    doc["sub-model"][i]["vertex"][j]["boneid"][1].GetUint();
                v.BoneID.z =
                    doc["sub-model"][i]["vertex"][j]["boneid"][2].GetUint();
                v.BoneID.w =
                    doc["sub-model"][i]["vertex"][j]["boneid"][3].GetUint();

                aniVertex.push_back(v);
            }
        }
        else
        {
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
        si.mVerteices = animated ? (void*)(&aniVertex) : (void*)(&vertex);
        std::vector<std::string> t = {};
        for (auto& tex : texture) { t.emplace_back(tex.mPath); }
        si.mTextures = &t;
        si.mStaticMaterial = "copper";
        si.mWithAnimation = animated;
        LAYOUT_TYPE layoutType = animated ?
            LAYOUT_TYPE::NORMAL_TANGENT_TEX_WEIGHT_BONE :
            LAYOUT_TYPE::NORMAL_TANGENT_TEX;
        GetRSRoot_DX11_Singleton()->MeshHelper()->ProcessSubMesh(_result,
            &si, layoutType);
    }
}

void AddDiffuseTexTo(RS_SUBMESH_DATA* _result, std::string _filePath)
{
    RSRoot_DX11* root = GetRSRoot_DX11_Singleton();

    static std::wstring wstr = L"";
    static std::string name = "";
    static HRESULT hr = S_OK;
    ID3D11ShaderResourceView* srv = nullptr;

    wstr = std::wstring(_filePath.begin(), _filePath.end());
    wstr = L".\\Assets\\Textures\\" + wstr;

    if (root->ResourceManager()->GetMeshSrv(_filePath))
    {
        if (_result->mTextures.size())
        {
            _result->mTextures[0] = _filePath;
        }
        else
        {
            _result->mTextures.emplace_back(_filePath);
        }
        return;
    }

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

    if (_result->mTextures.size())
    {
        _result->mTextures[0] = _filePath;
    }
    else
    {
        _result->mTextures.emplace_back(_filePath);
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

    if (root->ResourceManager()->GetMeshSrv(_filePath))
    {
        _result->mTextures.emplace_back(_filePath);
        return;
    }

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
