#include "ModelHelper.h"

#include "PrintLog.h"

#include <DDSTextureLoader11.h>
#include <RSDevices.h>
#include <RSMeshHelper.h>
#include <RSResourceManager.h>
#include <RSRoot_DX11.h>
#include <TextUtility.h>
#include <WICTextureLoader11.h>

#include <cstdio>
#include <cstring>
#include <fstream>

void loadByBinary(const std::string &FilePath,
                  RS_SUBMESH_DATA *OutResult,
                  int SubMeshIndex,
                  SUBMESH_BONES *OutBoneData,
                  MESH_ANIMATION_DATA **OutAnimData);
void loadByJson(const std::string &FilePath,
                RS_SUBMESH_DATA *OutResult,
                int SubMeshIndex,
                SUBMESH_BONES *OutBoneData,
                MESH_ANIMATION_DATA **OutAnimData);

void loadModelFile(const std::string &FilePath,
                   MODEL_FILE_TYPE Type,
                   int SubMeshIndex,
                   RS_SUBMESH_DATA *OutResult,
                   SUBMESH_BONES *OutBoneData,
                   MESH_ANIMATION_DATA **OutAnimData) {
  std::string Path = ".\\Assets\\Models\\" + FilePath;

  switch (Type) {
  case MODEL_FILE_TYPE::BIN:
    loadByBinary(Path, OutResult, SubMeshIndex, OutBoneData, OutAnimData);
    break;
  case MODEL_FILE_TYPE::JSON:
    loadByJson(Path, OutResult, SubMeshIndex, OutBoneData, OutAnimData);
    break;
  default:
    break;
  }
}

void loadByBinary(const std::string &FilePath,
                  RS_SUBMESH_DATA *OutResult,
                  int SubMeshIndex,
                  SUBMESH_BONES *OutBoneData,
                  MESH_ANIMATION_DATA **OutAnimData) {
  std::ifstream InFile(FilePath, std::ios::in | std::ios::binary);

#ifdef _DEBUG
  assert(InFile.is_open());
#endif // _DEBUG

  std::string Directory = "";
  std::string TexType = "";
  bool Animated = false;

  // directory
  {
    int Size = 0;
    char Str[128] = "";
    InFile.read(reinterpret_cast<char *>(&Size), sizeof(Size));
    InFile.read(Str, Size);
    Directory = Str;
  }

  // texture-type
  {
    int Size = 0;
    char Str[128] = "";
    InFile.read(reinterpret_cast<char *>(&Size), sizeof(Size));
    InFile.read(Str, Size);
    TexType = Str;
  }

  // animation-flag
  {
    int Flag = 0;
    InFile.read(reinterpret_cast<char *>(&Flag), sizeof(Flag));
    Animated = Flag ? true : false;
  }

  // sub-model-size
  int SubSize = 0;
  InFile.read(reinterpret_cast<char *>(&SubSize), sizeof(SubSize));
#ifdef _DEBUG
  assert(SubSize > SubMeshIndex);
#endif // _DEBUG

  if (Animated) {
    *OutAnimData = new MESH_ANIMATION_DATA;
    assert(*OutAnimData);
    MESH_NODE *RootNode = nullptr;
    static std::unordered_map<std::string, MESH_NODE *> NodeMap = {};
    static std::unordered_map<std::string, std::string> ParentMap = {};
    static std::unordered_map<std::string, std::vector<std::string>>
        ChildrenMap = {};
    NodeMap.clear();
    ParentMap.clear();
    ChildrenMap.clear();

    // node relationship
    int NodeSize = 0;
    InFile.read(reinterpret_cast<char *>(&NodeSize), sizeof(int));
    for (int I = 0; I < NodeSize; I++) {
      MESH_NODE *Node = new MESH_NODE;
      if (!RootNode) {
        RootNode = Node;
      }
      int NameLen = 0;
      char NodeName[1024] = "";
      InFile.read(reinterpret_cast<char *>(&NameLen), sizeof(int));
      InFile.read(NodeName, NameLen);
      Node->NodeName = NodeName;

      float Matrix[4][4] = {};
      for (UINT J = 0; J < 16; J++) {
        double TempD = 0.0;
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        int Fir = J / 4, Sec = J % 4;
        Matrix[Fir][Sec] = static_cast<float>(TempD);
      }

      Node->ThisToParent = dx::XMFLOAT4X4(
          Matrix[0][0], Matrix[0][1], Matrix[0][2], Matrix[0][3], Matrix[1][0],
          Matrix[1][1], Matrix[1][2], Matrix[1][3], Matrix[2][0], Matrix[2][1],
          Matrix[2][2], Matrix[2][3], Matrix[3][0], Matrix[3][1], Matrix[3][2],
          Matrix[3][3]);
      NodeMap.insert({NodeName, Node});

      int ParNameLen = 0;
      char ParName[1024] = "";
      InFile.read(reinterpret_cast<char *>(&ParNameLen), sizeof(int));
      InFile.read(ParName, ParNameLen);
      ParentMap.insert({NodeName, ParName});

      int ChildrenSize = 0;
      InFile.read(reinterpret_cast<char *>(&ChildrenSize), sizeof(int));
      ChildrenMap.insert({NodeName, {}});
      ChildrenMap[NodeName].resize(ChildrenSize);
      for (int J = 0; J < ChildrenSize; J++) {
        int ChdNameLen = 0;
        char ChdName[1024] = "";
        InFile.read(reinterpret_cast<char *>(&ChdNameLen), sizeof(int));
        InFile.read(ChdName, ChdNameLen);
        ChildrenMap[NodeName][J] = ChdName;
      }
    }

    for (auto &Node : NodeMap) {
      if (Node.second == RootNode) {
        Node.second->Parent = nullptr;
      } else {
        Node.second->Parent = NodeMap[ParentMap[Node.first]];
      }

      auto ChdSize = ChildrenMap[Node.first].size();
      Node.second->Children.resize(ChdSize);
      for (size_t I = 0; I < ChdSize; I++) {
        Node.second->Children[I] = NodeMap[ChildrenMap[Node.first][I]];
      }
    }

    (*OutAnimData)->RootNode = RootNode;

    // animations
    int AniSize = 0;
    InFile.read(reinterpret_cast<char *>(&AniSize), sizeof(int));
    (*OutAnimData)->AllAnimations.resize(AniSize);
    for (int I = 0; I < AniSize; I++) {
      auto &Ani = (*OutAnimData)->AllAnimations[I];

      int AniNameLen = 0;
      char AniName[1024] = "";
      InFile.read(reinterpret_cast<char *>(&AniNameLen), sizeof(int));
      InFile.read(AniName, AniNameLen);
      Ani.AnimationName = AniName;

      double Duration = 0.0;
      InFile.read(reinterpret_cast<char *>(&Duration), sizeof(double));
      Ani.Duration = static_cast<float>(Duration);

      double TicksPerSec = 0.0;
      InFile.read(reinterpret_cast<char *>(&TicksPerSec), sizeof(double));
      Ani.TicksPerSecond = static_cast<float>(TicksPerSec);

      int NodeActSize = 0;
      InFile.read(reinterpret_cast<char *>(&NodeActSize), sizeof(int));
      Ani.NodeActions.resize(NodeActSize);
      for (int J = 0; J < NodeActSize; J++) {
        auto &NodeAct = Ani.NodeActions[J];
        int NodeActNameLen = 0;
        char NodeActName[1024] = "";
        InFile.read(reinterpret_cast<char *>(&NodeActNameLen), sizeof(int));
        InFile.read(NodeActName, NodeActNameLen);
        NodeAct.NodeName = NodeActName;

        int PKSize = 0;
        InFile.read(reinterpret_cast<char *>(&PKSize), sizeof(int));
        NodeAct.PositionKeys.resize(PKSize);
        for (auto &PK : NodeAct.PositionKeys) {
          double TempD = 0.0;
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          PK.first = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          PK.second.x = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          PK.second.y = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          PK.second.z = static_cast<float>(TempD);
        }

        int RKSize = 0;
        InFile.read(reinterpret_cast<char *>(&RKSize), sizeof(int));
        NodeAct.RotationKeys.resize(RKSize);
        for (auto &RK : NodeAct.RotationKeys) {
          double TempD = 0.0;
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          RK.first = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          RK.second.x = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          RK.second.y = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          RK.second.z = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          RK.second.w = static_cast<float>(TempD);
        }

        int SKSize = 0;
        InFile.read(reinterpret_cast<char *>(&SKSize), sizeof(int));
        NodeAct.ScalingKeys.resize(SKSize);
        for (auto &SK : NodeAct.ScalingKeys) {
          double TempD = 0.0;
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          SK.first = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          SK.second.x = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          SK.second.y = static_cast<float>(TempD);
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          SK.second.z = static_cast<float>(TempD);
        }
      }
    }
  }

  std::vector<UINT> Index = {};
  std::vector<vertex_type::TangentVertex> Vertex = {};
  std::vector<vertex_type::AnimationVertex> AniVertex = {};
  std::vector<MODEL_TEXTURE_INFO> Texture = {};
  int IndexSize = 0;
  int VertexSize = 0;
  int TextureSize = 0;
  for (int I = 0; I < SubSize; I++) {
    Index.clear();
    Vertex.clear();
    AniVertex.clear();
    Texture.clear();

    // each-sub-sizes
    InFile.read(reinterpret_cast<char *>(&IndexSize), sizeof(IndexSize));
    InFile.read(reinterpret_cast<char *>(&VertexSize), sizeof(VertexSize));
    InFile.read(reinterpret_cast<char *>(&TextureSize), sizeof(TextureSize));

    // each-sub-index
    UINT Ind = 0;
    vertex_type::TangentVertex Ver = {};
    vertex_type::AnimationVertex AniVer = {};
    MODEL_TEXTURE_INFO Tex = {};
    for (int J = 0; J < IndexSize; J++) {
      InFile.read(reinterpret_cast<char *>(&Ind), sizeof(Ind));
      Index.push_back(Ind);
    }

    // each-sub-vertex
    if (!Animated) {
      double TempD = 0.0;
      for (int J = 0; J < VertexSize; J++) {
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.Position.x = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.Position.y = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.Position.z = static_cast<float>(TempD);

        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.Normal.x = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.Normal.y = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.Normal.z = static_cast<float>(TempD);

        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.Tangent.x = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.Tangent.y = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.Tangent.z = static_cast<float>(TempD);

        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.TexCoord.x = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        Ver.TexCoord.y = static_cast<float>(TempD);

        Vertex.push_back(Ver);
      }
    } else {
      double TempD = 0.0;
      UINT TempU = 0;
      for (int j = 0; j < VertexSize; j++) {
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Position.x = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Position.y = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Position.z = static_cast<float>(TempD);

        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Normal.x = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Normal.y = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Normal.z = static_cast<float>(TempD);

        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Tangent.x = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Tangent.y = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Tangent.z = static_cast<float>(TempD);

        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.TexCoord.x = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.TexCoord.y = static_cast<float>(TempD);

        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Weight.x = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Weight.y = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Weight.z = static_cast<float>(TempD);
        InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
        AniVer.Weight.w = static_cast<float>(TempD);

        InFile.read(reinterpret_cast<char *>(&TempU), sizeof(UINT));
        AniVer.BoneID.x = TempU;
        InFile.read(reinterpret_cast<char *>(&TempU), sizeof(UINT));
        AniVer.BoneID.y = TempU;
        InFile.read(reinterpret_cast<char *>(&TempU), sizeof(UINT));
        AniVer.BoneID.z = TempU;
        InFile.read(reinterpret_cast<char *>(&TempU), sizeof(UINT));
        AniVer.BoneID.w = TempU;

        AniVertex.push_back(AniVer);
      }
    }

    // each-sub-texture
    for (int J = 0; J < TextureSize; J++) {
      int Len = 0;
      char Str[1024] = "";
      InFile.read(reinterpret_cast<char *>(&Len), sizeof(Len));
      InFile.read(Str, Len);
      Tex.Type = Str;
      std::strcpy(Str, "");
      InFile.read(reinterpret_cast<char *>(&Len), sizeof(Len));
      InFile.read(Str, Len);
      Tex.Path = Str;
      Texture.push_back(Tex);
    }

    // each-sub-bone
    if (Animated) {
      int BoneSize = 0;
      InFile.read(reinterpret_cast<char *>(&BoneSize), sizeof(int));
      OutBoneData->resize(BoneSize);
      for (int J = 0; J < BoneSize; J++) {
        auto &Bone = (*OutBoneData)[J];
        int Len = 0;
        char BoneName[1024] = "";
        InFile.read(reinterpret_cast<char *>(&Len), sizeof(Len));
        InFile.read(BoneName, Len);
        Bone.BoneName = BoneName;
        float Matrix[4][4] = {};
        for (UINT K = 0; K < 16; K++) {
          double TempD = 0.0;
          InFile.read(reinterpret_cast<char *>(&TempD), sizeof(double));
          int Fir = K / 4, Sec = K % 4;
          Matrix[Fir][Sec] = static_cast<float>(TempD);
        }
        Bone.LocalToBone = dx::XMFLOAT4X4(
            Matrix[0][0], Matrix[0][1], Matrix[0][2], Matrix[0][3],
            Matrix[1][0], Matrix[1][1], Matrix[1][2], Matrix[1][3],
            Matrix[2][0], Matrix[2][1], Matrix[2][2], Matrix[2][3],
            Matrix[3][0], Matrix[3][1], Matrix[3][2], Matrix[3][3]);
        dx::XMStoreFloat4x4(&Bone.BoneTransform, dx::XMMatrixIdentity());
      }
    }

    if (I == SubMeshIndex) {
      break;
    }
  }

  SUBMESH_INFO SI = {};
  SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
  SI.IndeicesPtr = &Index;
  SI.VerteicesPtr =
      Animated ? static_cast<void *>(&AniVertex) : static_cast<void *>(&Vertex);
  std::vector<std::string> T = {};
  for (auto &Tex : Texture) {
    T.emplace_back(Tex.Path);
  }
  SI.TexturesPtr = &T;
  SI.AnimationFlag = Animated;
  LAYOUT_TYPE LayoutType = Animated
                               ? LAYOUT_TYPE::NORMAL_TANGENT_TEX_WEIGHT_BONE
                               : LAYOUT_TYPE::NORMAL_TANGENT_TEX;
  getRSDX11RootInstance()->getMeshHelper()->processSubMesh(OutResult, &SI,
                                                           LayoutType);

  InFile.close();
}

void loadByJson(const std::string &FilePath,
                RS_SUBMESH_DATA *OutResult,
                int SubMeshIndex,
                SUBMESH_BONES *OutBoneData,
                MESH_ANIMATION_DATA **OutAnimData) {
  std::FILE *FPtr = std::fopen(FilePath.c_str(), "rb");
  if (!FPtr) {
    P_LOG(LOG_ERROR, "cannt open file : {}", FilePath);
  }
  char *ReadBuf = new char[65536];
#ifdef _DEBUG
  assert(ReadBuf);
#endif // _DEBUG

  rapidjson::FileReadStream ReadStream(FPtr, ReadBuf, 65536 * sizeof(char));
  rapidjson::Document Doc = {};
  Doc.ParseStream(ReadStream);
  if (Doc.HasParseError()) {
    P_LOG(LOG_ERROR, "json error code : {}", Doc.GetParseError());
#ifdef _DEBUG
    assert(false && "json file invalid");
#endif // _DEBUG
  }
  delete[] ReadBuf;
  std::fclose(FPtr);

  std::string Directory = Doc["directory"].GetString();
  std::string TexType = Doc["texture-type"].GetString();
  bool Animated = Doc["with-animation"].GetBool();

  UINT SubSize = Doc["sub-model-size"].GetUint();
#ifdef _DEBUG
  assert(SubSize > static_cast<UINT>(SubMeshIndex));
#endif // _DEBUG
  UINT SubArraySize = Doc["sub-model"].Size();
#ifdef _DEBUG
  assert(SubSize == SubArraySize);
#endif // _DEBUG
  (void)SubArraySize;

  std::vector<UINT> Index = {};
  std::vector<vertex_type::TangentVertex> Vertex = {};
  std::vector<vertex_type::AnimationVertex> AniVertex = {};
  std::vector<MODEL_TEXTURE_INFO> Texture = {};
  for (UINT I = 0; I < SubSize; I++) {
    if (I != SubMeshIndex) {
      continue;
    }

    Index.clear();
    Vertex.clear();
    Texture.clear();
    UINT IndexSize = Doc["sub-model"][I]["index-size"].GetUint();
#ifdef _DEBUG
    assert(IndexSize == (UINT)Doc["sub-model"][I]["index"].Size());
#endif // _DEBUG
    UINT VertexSize = Doc["sub-model"][I]["vertex-size"].GetUint();
#ifdef _DEBUG
    assert(VertexSize == (UINT)Doc["sub-model"][I]["vertex"].Size());
#endif // _DEBUG
    UINT TexSize = Doc["sub-model"][I]["texture-size"].GetUint();
#ifdef _DEBUG
    assert(TexSize == (UINT)Doc["sub-model"][I]["texture"].Size());
#endif // _DEBUG

    for (UINT J = 0; J < IndexSize; J++) {
      Index.push_back(Doc["sub-model"][I]["index"][J].GetUint());
    }

    if (Animated) {
      for (UINT J = 0; J < VertexSize; J++) {
        static vertex_type::AnimationVertex V = {};
        V = {};

        V.Position.x =
            Doc["sub-model"][I]["vertex"][J]["position"][0].GetFloat();
        V.Position.y =
            Doc["sub-model"][I]["vertex"][J]["position"][1].GetFloat();
        V.Position.z =
            Doc["sub-model"][I]["vertex"][J]["position"][2].GetFloat();

        V.Normal.x = Doc["sub-model"][I]["vertex"][J]["normal"][0].GetFloat();
        V.Normal.y = Doc["sub-model"][I]["vertex"][J]["normal"][1].GetFloat();
        V.Normal.z = Doc["sub-model"][I]["vertex"][J]["normal"][2].GetFloat();

        V.Tangent.x = Doc["sub-model"][I]["vertex"][J]["tangent"][0].GetFloat();
        V.Tangent.y = Doc["sub-model"][I]["vertex"][J]["tangent"][1].GetFloat();
        V.Tangent.z = Doc["sub-model"][I]["vertex"][J]["tangent"][2].GetFloat();

        V.TexCoord.x =
            Doc["sub-model"][I]["vertex"][J]["texcoord"][0].GetFloat();
        V.TexCoord.y =
            Doc["sub-model"][I]["vertex"][J]["texcoord"][1].GetFloat();

        V.Weight.x = Doc["sub-model"][I]["vertex"][J]["weight"][0].GetFloat();
        V.Weight.y = Doc["sub-model"][I]["vertex"][J]["weight"][1].GetFloat();
        V.Weight.z = Doc["sub-model"][I]["vertex"][J]["weight"][2].GetFloat();
        V.Weight.w = Doc["sub-model"][I]["vertex"][J]["weight"][3].GetFloat();

        V.BoneID.x = Doc["sub-model"][I]["vertex"][J]["boneid"][0].GetUint();
        V.BoneID.y = Doc["sub-model"][I]["vertex"][J]["boneid"][1].GetUint();
        V.BoneID.z = Doc["sub-model"][I]["vertex"][J]["boneid"][2].GetUint();
        V.BoneID.w = Doc["sub-model"][I]["vertex"][J]["boneid"][3].GetUint();

        AniVertex.push_back(V);
      }
    } else {
      for (UINT J = 0; J < VertexSize; J++) {
        static vertex_type::TangentVertex V = {};
        V = {};

        V.Position.x =
            Doc["sub-model"][I]["vertex"][J]["position"][0].GetFloat();
        V.Position.y =
            Doc["sub-model"][I]["vertex"][J]["position"][1].GetFloat();
        V.Position.z =
            Doc["sub-model"][I]["vertex"][J]["position"][2].GetFloat();

        V.Normal.x = Doc["sub-model"][I]["vertex"][J]["normal"][0].GetFloat();
        V.Normal.y = Doc["sub-model"][I]["vertex"][J]["normal"][1].GetFloat();
        V.Normal.z = Doc["sub-model"][I]["vertex"][J]["normal"][2].GetFloat();

        V.Tangent.x = Doc["sub-model"][I]["vertex"][J]["tangent"][0].GetFloat();
        V.Tangent.y = Doc["sub-model"][I]["vertex"][J]["tangent"][1].GetFloat();
        V.Tangent.z = Doc["sub-model"][I]["vertex"][J]["tangent"][2].GetFloat();

        V.TexCoord.x =
            Doc["sub-model"][I]["vertex"][J]["texcoord"][0].GetFloat();
        V.TexCoord.y =
            Doc["sub-model"][I]["vertex"][J]["texcoord"][1].GetFloat();

        Vertex.push_back(V);
      }
    }

    for (UINT J = 0; J < TexSize; J++) {
      static MODEL_TEXTURE_INFO T = {};
      T = {};
      T.Type = Doc["sub-model"][I]["texture"][J]["type"].GetString();
      T.Path = Doc["sub-model"][I]["texture"][J]["path"].GetString();

      Texture.push_back(T);
    }

    if (Animated && OutBoneData) {
      UINT BoneSize = Doc["sub-model"][I]["bone"].Size();
      OutBoneData->resize(BoneSize);
      for (UINT J = 0; J < BoneSize; J++) {
        auto &Bone = (*OutBoneData)[J];
        Bone.BoneName = Doc["sub-model"][I]["bone"][J]["name"].GetString();
        float Matrix[4][4] = {};
        for (UINT K = 0; K < 16; K++) {
          int Fir = K / 4, Sec = K % 4;
          Matrix[Fir][Sec] = static_cast<float>(
              Doc["sub-model"][I]["bone"][J]["to-bone"][K].GetDouble());
        }
        Bone.LocalToBone = dx::XMFLOAT4X4(
            Matrix[0][0], Matrix[0][1], Matrix[0][2], Matrix[0][3],
            Matrix[1][0], Matrix[1][1], Matrix[1][2], Matrix[1][3],
            Matrix[2][0], Matrix[2][1], Matrix[2][2], Matrix[2][3],
            Matrix[3][0], Matrix[3][1], Matrix[3][2], Matrix[3][3]);
        dx::XMStoreFloat4x4(&Bone.BoneTransform, dx::XMMatrixIdentity());
      }
    }

    SUBMESH_INFO SI = {};
    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.IndeicesPtr = &Index;
    SI.VerteicesPtr = Animated ? static_cast<void *>(&AniVertex)
                               : static_cast<void *>(&Vertex);
    std::vector<std::string> T = {};
    for (auto &Tex : Texture) {
      T.emplace_back(Tex.Path);
    }
    SI.TexturesPtr = &T;
    SI.AnimationFlag = Animated;
    LAYOUT_TYPE LayoutType = Animated
                                 ? LAYOUT_TYPE::NORMAL_TANGENT_TEX_WEIGHT_BONE
                                 : LAYOUT_TYPE::NORMAL_TANGENT_TEX;
    getRSDX11RootInstance()->getMeshHelper()->processSubMesh(OutResult, &SI,
                                                             LayoutType);
  }

  if (Animated && OutAnimData) {
    UINT NodeSize = Doc["node-relationship"].Size();

    *OutAnimData = new MESH_ANIMATION_DATA;
    assert(*OutAnimData);
    MESH_NODE *RootNode = nullptr;
    static std::unordered_map<std::string, MESH_NODE *> NodeMap = {};
    NodeMap.clear();

    for (UINT I = 0; I < NodeSize; I++) {
      MESH_NODE *Node = new MESH_NODE;
      if (!RootNode) {
        RootNode = Node;
      }
      Node->NodeName = Doc["node-relationship"][I]["name"].GetString();
      float Matrix[4][4] = {};
      for (UINT J = 0; J < 16; J++) {
        int Fir = J / 4, Sec = J % 4;
        Matrix[Fir][Sec] = static_cast<float>(
            Doc["node-relationship"][I]["to-parent"][J].GetDouble());
      }
      Node->ThisToParent = dx::XMFLOAT4X4(
          Matrix[0][0], Matrix[0][1], Matrix[0][2], Matrix[0][3], Matrix[1][0],
          Matrix[1][1], Matrix[1][2], Matrix[1][3], Matrix[2][0], Matrix[2][1],
          Matrix[2][2], Matrix[2][3], Matrix[3][0], Matrix[3][1], Matrix[3][2],
          Matrix[3][3]);

      NodeMap.insert({Node->NodeName, Node});
    }

    for (UINT I = 0; I < NodeSize; I++) {
      std::string NodeName = Doc["node-relationship"][I]["name"].GetString();
      MESH_NODE *NodePtr = NodeMap[NodeName];
      assert(NodePtr);

      std::string ParentName =
          Doc["node-relationship"][I]["parent"].GetString();
      if (ParentName == "" && NodePtr == RootNode) {
        NodePtr->Parent = nullptr;
      } else {
        NodePtr->Parent = NodeMap[ParentName];
      }

      UINT ChildrenSize = Doc["node-relationship"][I]["children"].Size();
      NodePtr->Children.resize(ChildrenSize);
      for (UINT J = 0; J < ChildrenSize; J++) {
        std::string ChildName =
            Doc["node-relationship"][I]["children"][J].GetString();
        NodePtr->Children[J] = NodeMap[ChildName];
      }
    }

    (*OutAnimData)->RootNode = RootNode;

    UINT AnimationSize = Doc["animation"].Size();
    auto &AniVec = (*OutAnimData)->AllAnimations;
    AniVec.resize(AnimationSize);
    for (UINT AniIndex = 0; AniIndex < AnimationSize; AniIndex++) {
      auto &ThisAni = AniVec[AniIndex];
      ThisAni.AnimationName = Doc["animation"][AniIndex]["name"].GetString();
      ThisAni.Duration = static_cast<float>(
          Doc["animation"][AniIndex]["duration"].GetDouble());
      ThisAni.TicksPerSecond = static_cast<float>(
          Doc["animation"][AniIndex]["ticks-per-second"].GetDouble());

      UINT ThisNodeActionSize =
          Doc["animation"][AniIndex]["node-actions"].Size();
      ThisAni.NodeActions.resize(ThisNodeActionSize);
      for (UINT ActionIndex = 0; ActionIndex < ThisNodeActionSize;
           ActionIndex++) {
        auto &ThisChnl = ThisAni.NodeActions[ActionIndex];
        ThisChnl.NodeName =
            Doc["animation"][AniIndex]["node-actions"][ActionIndex]["node-name"]
                .GetString();

        UINT PosKeySize = Doc["animation"][AniIndex]["node-actions"]
                             [ActionIndex]["position-keys"]
                                 .Size();
        ThisChnl.PositionKeys.resize(PosKeySize);
        for (UINT PKIndex = 0; PKIndex < PosKeySize; PKIndex++) {
          ThisChnl.PositionKeys[PKIndex].first = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["position-keys"][PKIndex]["time"]
                     .GetDouble());
          ThisChnl.PositionKeys[PKIndex].second.x = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["position-keys"][PKIndex]["value"][0]
                     .GetDouble());
          ThisChnl.PositionKeys[PKIndex].second.y = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["position-keys"][PKIndex]["value"][1]
                     .GetDouble());
          ThisChnl.PositionKeys[PKIndex].second.z = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["position-keys"][PKIndex]["value"][2]
                     .GetDouble());
        }

        UINT RotKeySize = Doc["animation"][AniIndex]["node-actions"]
                             [ActionIndex]["rotation-keys"]
                                 .Size();
        ThisChnl.RotationKeys.resize(RotKeySize);
        for (UINT RKIndex = 0; RKIndex < RotKeySize; RKIndex++) {
          ThisChnl.RotationKeys[RKIndex].first = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["rotation-keys"][RKIndex]["time"]
                     .GetDouble());
          ThisChnl.RotationKeys[RKIndex].second.x = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["rotation-keys"][RKIndex]["value"][0]
                     .GetDouble());
          ThisChnl.RotationKeys[RKIndex].second.y = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["rotation-keys"][RKIndex]["value"][1]
                     .GetDouble());
          ThisChnl.RotationKeys[RKIndex].second.z = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["rotation-keys"][RKIndex]["value"][2]
                     .GetDouble());
          ThisChnl.RotationKeys[RKIndex].second.w = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["rotation-keys"][RKIndex]["value"][3]
                     .GetDouble());
        }

        UINT ScaKeySize = Doc["animation"][AniIndex]["node-actions"]
                             [ActionIndex]["scaling-keys"]
                                 .Size();
        ThisChnl.ScalingKeys.resize(ScaKeySize);
        for (UINT SKIndex = 0; SKIndex < ScaKeySize; SKIndex++) {
          ThisChnl.ScalingKeys[SKIndex].first = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["scaling-keys"][SKIndex]["time"]
                     .GetDouble());
          ThisChnl.ScalingKeys[SKIndex].second.x = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["scaling-keys"][SKIndex]["value"][0]
                     .GetDouble());
          ThisChnl.ScalingKeys[SKIndex].second.y = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["scaling-keys"][SKIndex]["value"][1]
                     .GetDouble());
          ThisChnl.ScalingKeys[SKIndex].second.z = static_cast<float>(
              Doc["animation"][AniIndex]["node-actions"][ActionIndex]
                 ["scaling-keys"][SKIndex]["value"][2]
                     .GetDouble());
        }
      }
    }
  }
}

void addTextureToSubMesh(RS_SUBMESH_DATA *OutResult,
                         const std::string &FilePath,
                         MESH_TEXTURE_TYPE Type) {
  RSRoot_DX11 *Root = getRSDX11RootInstance();

  std::wstring WStr = L"";
  std::string Name = "";
  HRESULT Hr = S_OK;
  ID3D11ShaderResourceView *Srv = nullptr;

  WStr = std::wstring(FilePath.begin(), FilePath.end());
  WStr = L".\\Assets\\Textures\\" + WStr;

  if (Root->getResourceManager()->getMeshSrv(FilePath)) {
    OutResult->Textures[static_cast<size_t>(Type)] = FilePath;
    return;
  }

  if (FilePath.find(".dds") != std::string::npos ||
      FilePath.find(".DDS") != std::string::npos) {
    Hr = dx::CreateDDSTextureFromFile(Root->getDevices()->getDevice(),
                                      WStr.c_str(), nullptr, &Srv);
    if (SUCCEEDED(Hr)) {
      Name = FilePath;
      Root->getResourceManager()->addMeshSrv(Name, Srv);
    } else {
      P_LOG(LOG_ERROR, "failed to load texture : {}", FilePath);
    }
  } else {
    Hr = dx::CreateWICTextureFromFile(Root->getDevices()->getDevice(),
                                      WStr.c_str(), nullptr, &Srv);
    if (SUCCEEDED(Hr)) {
      Name = FilePath;
      Root->getResourceManager()->addMeshSrv(Name, Srv);
    } else {
      P_LOG(LOG_ERROR, "failed to load texture : {}", FilePath);
    }
  }

  OutResult->Textures[static_cast<size_t>(Type)] = FilePath;
}
