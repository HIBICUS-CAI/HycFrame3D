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

void LoadByBinary(const std::string _filePath, RS_SUBMESH_DATA* _result,
    int _subMeshIndex, SUBMESH_BONES* _boneData,
    MESH_ANIMATION_DATA** _animData);
void LoadByJson(const std::string _filePath, RS_SUBMESH_DATA* _result,
    int _subMeshIndex, SUBMESH_BONES* _boneData,
    MESH_ANIMATION_DATA** _animData);

void LoadModelFile(const std::string _filePath, MODEL_FILE_TYPE _type,
    int _subMeshIndex, RS_SUBMESH_DATA* _result,
    SUBMESH_BONES* _boneData, MESH_ANIMATION_DATA** _animData)
{
    static std::string path = "";
    path = ".\\Assets\\Models\\" + _filePath;

    switch (_type)
    {
    case MODEL_FILE_TYPE::BIN:
        LoadByBinary(path, _result, _subMeshIndex, _boneData, _animData); break;
    case MODEL_FILE_TYPE::JSON:
        LoadByJson(path, _result, _subMeshIndex, _boneData, _animData); break;
    default:
        break;
    }
}

void LoadByBinary(const std::string _filePath, RS_SUBMESH_DATA* _result,
    int _subMeshIndex, SUBMESH_BONES* _boneData,
    MESH_ANIMATION_DATA** _animData)
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
    assert(subSize > _subMeshIndex);
#endif // _DEBUG

    if (animated)
    {
        *_animData = new MESH_ANIMATION_DATA;
        assert(*_animData);
        MESH_NODE* rootNode = nullptr;
        static std::unordered_map<std::string, MESH_NODE*> nodeMap = {};
        static std::unordered_map<std::string, std::string> parentMap = {};
        static std::unordered_map<std::string, std::vector<std::string>> childrenMap = {};
        nodeMap.clear();
        parentMap.clear();
        childrenMap.clear();

        // node relationship
        int nodeSize = 0;
        inFile.read((char*)(&nodeSize), sizeof(int));
        for (int i = 0; i < nodeSize; i++)
        {
            MESH_NODE* node = new MESH_NODE;
            if (!rootNode) { rootNode = node; }
            int nameLen = 0;
            char nodeName[1024] = "";
            inFile.read((char*)(&nameLen), sizeof(int));
            inFile.read(nodeName, nameLen);
            node->mNodeName = nodeName;

            float mat[4][4] = {};
            for (UINT j = 0; j < 16; j++)
            {
                double tempD = 0.0;
                inFile.read((char*)(&tempD), sizeof(double));
                int fir = j / 4, sec = j % 4;
                mat[fir][sec] = (float)tempD;
            }

            node->mThisToParent = DirectX::XMFLOAT4X4(
                mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
            nodeMap.insert({ nodeName,node });

            int parNameLen = 0;
            char parName[1024] = "";
            inFile.read((char*)(&parNameLen), sizeof(int));
            inFile.read(parName, parNameLen);
            parentMap.insert({ nodeName,parName });

            int childrenSize = 0;
            inFile.read((char*)(&childrenSize), sizeof(int));
            childrenMap.insert({ nodeName,{} });
            childrenMap[nodeName].resize(childrenSize);
            for (int j = 0; j < childrenSize; j++)
            {
                int chdNameLen = 0;
                char chdName[1024] = "";
                inFile.read((char*)(&chdNameLen), sizeof(int));
                inFile.read(chdName, chdNameLen);
                childrenMap[nodeName][j] = chdName;
            }
        }

        for (auto& node : nodeMap)
        {
            if (node.second == rootNode)
            {
                node.second->mParent = nullptr;
            }
            else
            {
                node.second->mParent = nodeMap[parentMap[node.first]];
            }

            auto chdSize = childrenMap[node.first].size();
            node.second->mChildren.resize(chdSize);
            for (size_t i = 0; i < chdSize; i++)
            {
                node.second->mChildren[i] = nodeMap[childrenMap[node.first][i]];
            }
        }

        (*_animData)->mRootNode = rootNode;

        // animations
        int aniSize = 0;
        inFile.read((char*)(&aniSize), sizeof(int));
        (*_animData)->mAllAnimations.resize(aniSize);
        for (int i = 0; i < aniSize; i++)
        {
            auto& ani = (*_animData)->mAllAnimations[i];

            int aniNameLen = 0;
            char aniName[1024] = "";
            inFile.read((char*)(&aniNameLen), sizeof(int));
            inFile.read(aniName, aniNameLen);
            ani.mAnimationName = aniName;

            double duration = 0.0;
            inFile.read((char*)(&duration), sizeof(double));
            ani.mDuration = (float)duration;

            double ticksPerSec = 0.0;
            inFile.read((char*)(&ticksPerSec), sizeof(double));
            ani.mTicksPerSecond = (float)ticksPerSec;

            int nodeActSize = 0;
            inFile.read((char*)(&nodeActSize), sizeof(int));
            ani.mNodeActions.resize(nodeActSize);
            for (int j = 0; j < nodeActSize; j++)
            {
                auto& nodeAct = ani.mNodeActions[j];
                int nodeActNameLen = 0;
                char nodeActName[1024] = "";
                inFile.read((char*)(&nodeActNameLen), sizeof(int));
                inFile.read(nodeActName, nodeActNameLen);
                nodeAct.mNodeName = nodeActName;

                int pkSize = 0;
                inFile.read((char*)(&pkSize), sizeof(int));
                nodeAct.mPositionKeys.resize(pkSize);
                for (auto& pk : nodeAct.mPositionKeys)
                {
                    double tempD = 0.0;
                    inFile.read((char*)(&tempD), sizeof(double));
                    pk.first = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    pk.second.x = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    pk.second.y = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    pk.second.z = (float)tempD;
                }

                int rkSize = 0;
                inFile.read((char*)(&rkSize), sizeof(int));
                nodeAct.mRotationKeys.resize(rkSize);
                for (auto& rk : nodeAct.mRotationKeys)
                {
                    double tempD = 0.0;
                    inFile.read((char*)(&tempD), sizeof(double));
                    rk.first = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    rk.second.x = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    rk.second.y = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    rk.second.z = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    rk.second.w = (float)tempD;
                }

                int skSize = 0;
                inFile.read((char*)(&skSize), sizeof(int));
                nodeAct.mScalingKeys.resize(skSize);
                for (auto& sk : nodeAct.mScalingKeys)
                {
                    double tempD = 0.0;
                    inFile.read((char*)(&tempD), sizeof(double));
                    sk.first = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    sk.second.x = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    sk.second.y = (float)tempD;
                    inFile.read((char*)(&tempD), sizeof(double));
                    sk.second.z = (float)tempD;
                }
            }
        }
    }

    std::vector<UINT> index = {};
    std::vector<VertexType::TangentVertex> vertex = {};
    std::vector<VertexType::AnimationVertex> aniVertex = {};
    std::vector<MODEL_TEXTURE_INFO> texture = {};
    int indexSize = 0;
    int vertexSize = 0;
    int textureSize = 0;
    for (int i = 0; i < subSize; i++)
    {
        index.clear();
        vertex.clear();
        aniVertex.clear();
        texture.clear();

        // each-sub-sizes
        inFile.read((char*)&indexSize, sizeof(indexSize));
        inFile.read((char*)&vertexSize, sizeof(vertexSize));
        inFile.read((char*)&textureSize, sizeof(textureSize));

        // each-sub-index
        UINT ind = 0;
        VertexType::TangentVertex ver = {};
        VertexType::AnimationVertex aniVer = {};
        MODEL_TEXTURE_INFO tex = {};
        for (int j = 0; j < indexSize; j++)
        {
            inFile.read((char*)&ind, sizeof(ind));
            index.push_back(ind);
        }

        // each-sub-vertex
        if (!animated)
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
        else
        {
            double temp = 0.0;
            UINT tempUI = 0;
            for (int j = 0; j < vertexSize; j++)
            {
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Position.x = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Position.y = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Position.z = (float)temp;

                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Normal.x = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Normal.y = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Normal.z = (float)temp;

                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Tangent.x = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Tangent.y = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Tangent.z = (float)temp;

                inFile.read((char*)(&temp), sizeof(double));
                aniVer.TexCoord.x = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.TexCoord.y = (float)temp;

                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Weight.x = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Weight.y = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Weight.z = (float)temp;
                inFile.read((char*)(&temp), sizeof(double));
                aniVer.Weight.w = (float)temp;

                inFile.read((char*)(&tempUI), sizeof(UINT));
                aniVer.BoneID.x = tempUI;
                inFile.read((char*)(&tempUI), sizeof(UINT));
                aniVer.BoneID.y = tempUI;
                inFile.read((char*)(&tempUI), sizeof(UINT));
                aniVer.BoneID.z = tempUI;
                inFile.read((char*)(&tempUI), sizeof(UINT));
                aniVer.BoneID.w = tempUI;

                aniVertex.push_back(aniVer);
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

        // each-sub-bone
        if (animated)
        {
            int boneSize = 0;
            inFile.read((char*)(&boneSize), sizeof(int));
            _boneData->resize(boneSize);
            for (int j = 0; j < boneSize; j++)
            {
                auto& bone = (*_boneData)[j];
                int len = 0;
                char boneName[1024] = "";
                inFile.read((char*)(&len), sizeof(len));
                inFile.read(boneName, len);
                bone.mBoneName = boneName;
                float mat[4][4] = {};
                for (UINT k = 0; k < 16; k++)
                {
                    double tempD = 0.0;
                    inFile.read((char*)(&tempD), sizeof(double));
                    int fir = k / 4, sec = k % 4;
                    mat[fir][sec] = (float)tempD;
                }
                bone.mLocalToBone = DirectX::XMFLOAT4X4(
                    mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                    mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                    mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                    mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
                DirectX::XMStoreFloat4x4(&bone.mBoneTransform,
                    DirectX::XMMatrixIdentity());
            }
        }

        if (i == _subMeshIndex) { break; }
    }

    SUBMESH_INFO si = {};
    si.mTopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    si.mIndeices = &index;
    si.mVerteices = animated ? (void*)(&aniVertex) : (void*)(&vertex);
    std::vector<std::string> t = {};
    for (auto& tex : texture) { t.emplace_back(tex.mPath); }
    si.mTextures = &t;
    si.mWithAnimation = animated;
    LAYOUT_TYPE layoutType = animated ?
        LAYOUT_TYPE::NORMAL_TANGENT_TEX_WEIGHT_BONE :
        LAYOUT_TYPE::NORMAL_TANGENT_TEX;
    GetRSRoot_DX11_Singleton()->MeshHelper()->ProcessSubMesh(_result,
        &si, layoutType);

    inFile.close();
}

void LoadByJson(const std::string _filePath, RS_SUBMESH_DATA* _result,
    int _subMeshIndex, SUBMESH_BONES* _boneData,
    MESH_ANIMATION_DATA** _animData)
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
    assert(subSize > (UINT)_subMeshIndex);
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
        if (i != _subMeshIndex) { continue; }

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

        if (animated && _boneData)
        {
            UINT boneSize = doc["sub-model"][i]["bone"].Size();
            _boneData->resize(boneSize);
            for (UINT j = 0; j < boneSize; j++)
            {
                auto& bone = (*_boneData)[j];
                bone.mBoneName =
                    doc["sub-model"][i]["bone"][j]["name"].GetString();
                float mat[4][4] = {};
                for (UINT k = 0; k < 16; k++)
                {
                    int fir = k / 4, sec = k % 4;
                    mat[fir][sec] =
                        (float)(doc["sub-model"][i]["bone"][j]["to-bone"][k].
                            GetDouble());
                }
                bone.mLocalToBone = DirectX::XMFLOAT4X4(
                    mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                    mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                    mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                    mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
                DirectX::XMStoreFloat4x4(&bone.mBoneTransform,
                    DirectX::XMMatrixIdentity());
            }
        }

        SUBMESH_INFO si = {};
        si.mTopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
        si.mIndeices = &index;
        si.mVerteices = animated ? (void*)(&aniVertex) : (void*)(&vertex);
        std::vector<std::string> t = {};
        for (auto& tex : texture) { t.emplace_back(tex.mPath); }
        si.mTextures = &t;
        si.mWithAnimation = animated;
        LAYOUT_TYPE layoutType = animated ?
            LAYOUT_TYPE::NORMAL_TANGENT_TEX_WEIGHT_BONE :
            LAYOUT_TYPE::NORMAL_TANGENT_TEX;
        GetRSRoot_DX11_Singleton()->MeshHelper()->ProcessSubMesh(_result,
            &si, layoutType);
    }

    if (animated && _animData)
    {
        UINT nodeSize = doc["node-relationship"].Size();

        *_animData = new MESH_ANIMATION_DATA;
        assert(*_animData);
        MESH_NODE* rootNode = nullptr;
        static std::unordered_map<std::string, MESH_NODE*> nodeMap = {};
        nodeMap.clear();

        for (UINT i = 0; i < nodeSize; i++)
        {
            MESH_NODE* node = new MESH_NODE;
            if (!rootNode) { rootNode = node; }
            node->mNodeName =
                doc["node-relationship"][i]["name"].GetString();
            float mat[4][4] = {};
            for (UINT j = 0; j < 16; j++)
            {
                int fir = j / 4, sec = j % 4;
                mat[fir][sec] = (float)(
                    doc["node-relationship"][i]["to-parent"][j].
                    GetDouble());
            }
            node->mThisToParent = DirectX::XMFLOAT4X4(
                mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                mat[3][0], mat[3][1], mat[3][2], mat[3][3]);

            nodeMap.insert({ node->mNodeName,node });
        }

        for (UINT i = 0; i < nodeSize; i++)
        {
            std::string nodeName =
                doc["node-relationship"][i]["name"].GetString();
            MESH_NODE* nodePtr = nodeMap[nodeName];
            assert(nodePtr);

            std::string parentName =
                doc["node-relationship"][i]["parent"].GetString();
            if (parentName == "" && nodePtr == rootNode)
            {
                nodePtr->mParent = nullptr;
            }
            else
            {
                nodePtr->mParent = nodeMap[parentName];
            }

            UINT childrenSize =
                doc["node-relationship"][i]["children"].Size();
            nodePtr->mChildren.resize(childrenSize);
            for (UINT j = 0; j < childrenSize; j++)
            {
                std::string childName =
                    doc["node-relationship"][i]["children"][j].
                    GetString();
                nodePtr->mChildren[j] = nodeMap[childName];
            }
        }

        (*_animData)->mRootNode = rootNode;

        UINT animationSize = doc["animation"].Size();
        auto& aniVec = (*_animData)->mAllAnimations;
        aniVec.resize(animationSize);
        for (UINT aniIndex = 0; aniIndex < animationSize; aniIndex++)
        {
            auto& thisAni = aniVec[aniIndex];
            thisAni.mAnimationName =
                doc["animation"][aniIndex]["name"].GetString();
            thisAni.mDuration = (float)
                doc["animation"][aniIndex]["duration"].GetDouble();
            thisAni.mTicksPerSecond = (float)
                doc["animation"][aniIndex]["ticks-per-second"].GetDouble();

            UINT thisNodeActionSize =
                doc["animation"][aniIndex]["node-actions"].Size();
            thisAni.mNodeActions.resize(thisNodeActionSize);
            for (UINT actionIndex = 0;
                actionIndex < thisNodeActionSize; actionIndex++)
            {
                auto& thisChnl = thisAni.mNodeActions[actionIndex];
                thisChnl.mNodeName =
                    doc["animation"][aniIndex]["node-actions"]
                    [actionIndex]["node-name"].GetString();

                UINT posKeySize = doc["animation"][aniIndex]
                    ["node-actions"][actionIndex]["position-keys"].
                    Size();
                thisChnl.mPositionKeys.resize(posKeySize);
                for (UINT pkIndex = 0; pkIndex < posKeySize; pkIndex++)
                {
                    thisChnl.mPositionKeys[pkIndex].first = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["position-keys"][pkIndex]["time"].GetDouble();
                    thisChnl.mPositionKeys[pkIndex].second.x = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["position-keys"][pkIndex]
                        ["value"][0].GetDouble();
                    thisChnl.mPositionKeys[pkIndex].second.y = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["position-keys"][pkIndex]
                        ["value"][1].GetDouble();
                    thisChnl.mPositionKeys[pkIndex].second.z = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["position-keys"][pkIndex]
                        ["value"][2].GetDouble();
                }

                UINT rotKeySize = doc["animation"][aniIndex]
                    ["node-actions"][actionIndex]["rotation-keys"].
                    Size();
                thisChnl.mRotationKeys.resize(rotKeySize);
                for (UINT rkIndex = 0; rkIndex < rotKeySize; rkIndex++)
                {
                    thisChnl.mRotationKeys[rkIndex].first = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["rotation-keys"][rkIndex]["time"].GetDouble();
                    thisChnl.mRotationKeys[rkIndex].second.x = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["rotation-keys"][rkIndex]
                        ["value"][0].GetDouble();
                    thisChnl.mRotationKeys[rkIndex].second.y = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["rotation-keys"][rkIndex]
                        ["value"][1].GetDouble();
                    thisChnl.mRotationKeys[rkIndex].second.z = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["rotation-keys"][rkIndex]
                        ["value"][2].GetDouble();
                    thisChnl.mRotationKeys[rkIndex].second.w = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["rotation-keys"][rkIndex]
                        ["value"][3].GetDouble();
                }

                UINT scaKeySize = doc["animation"][aniIndex]
                    ["node-actions"][actionIndex]["scaling-keys"].
                    Size();
                thisChnl.mScalingKeys.resize(scaKeySize);
                for (UINT skIndex = 0; skIndex < scaKeySize; skIndex++)
                {
                    thisChnl.mScalingKeys[skIndex].first = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["scaling-keys"][skIndex]["time"].GetDouble();
                    thisChnl.mScalingKeys[skIndex].second.x = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["scaling-keys"][skIndex]
                        ["value"][0].GetDouble();
                    thisChnl.mScalingKeys[skIndex].second.y = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["scaling-keys"][skIndex]
                        ["value"][1].GetDouble();
                    thisChnl.mScalingKeys[skIndex].second.z = (float)
                        doc["animation"][aniIndex]
                        ["node-actions"][actionIndex]
                        ["scaling-keys"][skIndex]
                        ["value"][2].GetDouble();
                }
            }
        }
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
