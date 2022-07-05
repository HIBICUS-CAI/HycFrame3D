//---------------------------------------------------------------
// File: RSMeshHelper.cpp
// Proj: RenderSystem_DX11
// Info: 提供对SubMesh转换为dx可识别形式的方法
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSMeshHelper.h"

#include "DDSTextureLoader11.h"
#include "RSDevices.h"
#include "RSResourceManager.h"
#include "RSRoot_DX11.h"
#include "RSStaticResources.h"
#include "WICTextureLoader11.h"

#include <cassert>

using namespace dx;

RSMeshHelper::RSMeshHelper()
    : RenderSystemRoot(nullptr), ResourceManager(nullptr), Devices(nullptr),
      GeoGenerator(nullptr) {}

RSMeshHelper::~RSMeshHelper() {}

bool
RSMeshHelper::startUp(RSRoot_DX11 *RootPtr, RSResourceManager *ResManagerPtr) {
  if (!RootPtr || !ResManagerPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;
  ResourceManager = ResManagerPtr;
  Devices = RootPtr->getDevices();
  GeoGenerator = new RSGeometryGenerator(RootPtr);

  return true;
}

void
RSMeshHelper::cleanAndStop() {
  if (GeoGenerator) {
    delete GeoGenerator;
    GeoGenerator = nullptr;
  }
}

RSGeometryGenerator *
RSMeshHelper::getGeoGenerator() {
  return GeoGenerator;
}

void
RSMeshHelper::processSubMesh(RS_SUBMESH_DATA *OutResult,
                             const SUBMESH_INFO *Info,
                             LAYOUT_TYPE LayoutType) {
  assert(Info);

  auto TopologyType = Info->TopologyType;
  switch (TopologyType) {
  case TOPOLOGY_TYPE::POINTLIST:
    OutResult->TopologyType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    break;
  case TOPOLOGY_TYPE::LINELIST:
    OutResult->TopologyType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    break;
  case TOPOLOGY_TYPE::LINESTRIP:
    OutResult->TopologyType = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
    break;
  case TOPOLOGY_TYPE::TRIANGLELIST:
    OutResult->TopologyType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    break;
  case TOPOLOGY_TYPE::TRIANGLESTRIP:
    OutResult->TopologyType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    break;
  default:
    OutResult->TopologyType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    break;
  }
  OutResult->AnimationFlag = Info->AnimationFlag;
  OutResult->InputLayout = refStaticInputLayout(LayoutType);
  OutResult->IndexBuffer = createIndexBuffer(Info->IndeicesPtr);
  OutResult->VertexBuffer = createVertexBuffer(Info->VerteicesPtr, LayoutType);
  OutResult->IndexSize = static_cast<UINT>(Info->IndeicesPtr->size());
  createTexSrv(OutResult, Info->TexturesPtr);
  if (Info->MaterialPtr) {
    createSubMeshMaterial(OutResult, Info->MaterialPtr);
  }
}

ID3D11InputLayout *
RSMeshHelper::refStaticInputLayout(LAYOUT_TYPE LayoutType) {
  ID3D11InputLayout *InputLayout = nullptr;
  std::string Name = "";
  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR:
    Name = "ColorVertex";
    break;
  case LAYOUT_TYPE::NORMAL_TEX:
    Name = "BasicVertex";
    break;
  case LAYOUT_TYPE::NORMAL_TANGENT_TEX:
    Name = "TangentVertex";
    break;
  case LAYOUT_TYPE::NORMAL_TANGENT_TEX_WEIGHT_BONE:
    Name = "AnimationVertex";
    break;
  default:
    return nullptr;
  }
  InputLayout =
      RenderSystemRoot->getStaticResources()->getStaticInputLayout(Name);

  return InputLayout;
}

ID3D11Buffer *
RSMeshHelper::createIndexBuffer(const std::vector<UINT> *const IndicesArray) {
  ID3D11Buffer *IndexBuffer = nullptr;
  D3D11_BUFFER_DESC IBD = {};
  D3D11_SUBRESOURCE_DATA InitData = {};
  ZeroMemory(&IBD, sizeof(IBD));
  ZeroMemory(&InitData, sizeof(InitData));
  IBD.Usage = D3D11_USAGE_IMMUTABLE;
  IBD.ByteWidth = static_cast<UINT>(sizeof(UINT) * IndicesArray->size());
  IBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
  IBD.CPUAccessFlags = 0;
  IBD.MiscFlags = 0;

  InitData.pSysMem = IndicesArray->data();

  HRESULT Hr = S_OK;
  Hr = Devices->getDevice()->CreateBuffer(&IBD, &InitData, &IndexBuffer);
  if (SUCCEEDED(Hr)) {
    return IndexBuffer;
  } else {
    return nullptr;
  }
}

ID3D11Buffer *
RSMeshHelper::createVertexBuffer(const void *const ConstRawVerticesPtr,
                                 LAYOUT_TYPE LayoutType) {
  if (!ConstRawVerticesPtr) {
    return nullptr;
  }
  void *RawVerticesPtr = const_cast<void *>(ConstRawVerticesPtr);
  std::vector<vertex_type::BasicVertex> *BasicPtr = nullptr;
  std::vector<vertex_type::ColorVertex> *ColorPtr = nullptr;
  std::vector<vertex_type::TangentVertex> *TangentPtr = nullptr;
  std::vector<vertex_type::AnimationVertex> *AnimatedPtr = nullptr;
  UINT Size = 0;
  UINT VertexSize = 0;
  void *VertArray = nullptr;

  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR:
    ColorPtr =
        static_cast<std::vector<vertex_type::ColorVertex> *>(RawVerticesPtr);
    Size = static_cast<UINT>(ColorPtr->size());
    VertexSize = static_cast<UINT>(sizeof(vertex_type::ColorVertex));
    VertArray = ColorPtr->data();
    break;
  case LAYOUT_TYPE::NORMAL_TEX:
    BasicPtr =
        static_cast<std::vector<vertex_type::BasicVertex> *>(RawVerticesPtr);
    Size = static_cast<UINT>(BasicPtr->size());
    VertexSize = static_cast<UINT>(sizeof(vertex_type::BasicVertex));
    VertArray = BasicPtr->data();
    break;
  case LAYOUT_TYPE::NORMAL_TANGENT_TEX:
    TangentPtr =
        static_cast<std::vector<vertex_type::TangentVertex> *>(RawVerticesPtr);
    Size = static_cast<UINT>(TangentPtr->size());
    VertexSize = static_cast<UINT>(sizeof(vertex_type::TangentVertex));
    VertArray = TangentPtr->data();
    break;
  case LAYOUT_TYPE::NORMAL_TANGENT_TEX_WEIGHT_BONE:
    AnimatedPtr = static_cast<std::vector<vertex_type::AnimationVertex> *>(
        RawVerticesPtr);
    Size = static_cast<UINT>(AnimatedPtr->size());
    VertexSize = static_cast<UINT>(sizeof(vertex_type::AnimationVertex));
    VertArray = AnimatedPtr->data();
    break;
  default:
    return nullptr;
  }

  ID3D11Buffer *VertexBuffer = nullptr;
  D3D11_BUFFER_DESC VBD = {};
  D3D11_SUBRESOURCE_DATA InitData = {};
  ZeroMemory(&VBD, sizeof(VBD));
  ZeroMemory(&InitData, sizeof(InitData));
  VBD.Usage = D3D11_USAGE_IMMUTABLE;
  VBD.ByteWidth = VertexSize * Size;
  VBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  VBD.CPUAccessFlags = 0;
  VBD.MiscFlags = 0;

  InitData.pSysMem = VertArray;

  HRESULT Hr = S_OK;
  Hr = Devices->getDevice()->CreateBuffer(&VBD, &InitData, &VertexBuffer);
  if (SUCCEEDED(Hr)) {
    return VertexBuffer;
  } else {
    return nullptr;
  }
}

void
RSMeshHelper::createTexSrv(RS_SUBMESH_DATA *OutResult,
                           const std::vector<std::string> *const Textures) {
  auto &TexArrat = OutResult->Textures;
  static std::wstring WStr = L"";
  static std::string Name = "";
  static HRESULT Hr = S_OK;
  ID3D11ShaderResourceView *Srv = nullptr;

  for (auto &Tex : *Textures) {
    Name = Tex;
    auto ExistedSrv = ResourceManager->getMeshSrv(Name);
    if (ExistedSrv) {
      TexArrat[0] = Name;
      continue;
    }

    WStr = std::wstring(Tex.begin(), Tex.end());
    WStr = L".\\Assets\\Textures\\" + WStr;
    if (Tex.find(".dds") != std::string::npos ||
        Tex.find(".DDS") != std::string::npos) {
      Hr = dx::CreateDDSTextureFromFile(Devices->getDevice(), WStr.c_str(),
                                        nullptr, &Srv);
      if (SUCCEEDED(Hr)) {
        Name = Tex;
        ResourceManager->addMeshSrv(Name, Srv);
        TexArrat[0] = Name;
      } else {
        char ErrorLog[128] = "";
        sprintf_s(ErrorLog, 128,
                  "[[[WARNING]]] : cannot load this texture %s\n",
                  Name.c_str());
        OutputDebugString(ErrorLog);
      }
    } else {
      Hr = dx::CreateWICTextureFromFile(Devices->getDevice(), WStr.c_str(),
                                        nullptr, &Srv);
      if (SUCCEEDED(Hr)) {
        Name = Tex;
        ResourceManager->addMeshSrv(Name, Srv);
        TexArrat[0] = Name;
      } else {
        char ErrorLog[128] = "";
        sprintf_s(ErrorLog, 128,
                  "[[[WARNING]]] : cannot load this texture %s\n",
                  Name.c_str());
        OutputDebugString(ErrorLog);
      }
    }
  }
}

void
RSMeshHelper::createSubMeshMaterial(RS_SUBMESH_DATA *OutResult,
                                    const MATERIAL_INFO *const Info) {
  assert(Info);

  RS_MATERIAL_INFO *MaterialPtr = &(OutResult->Material);
  memcpy_s(MaterialPtr, sizeof(RS_MATERIAL_INFO), Info, sizeof(MATERIAL_INFO));
}

void
RSMeshHelper::releaseSubMesh(RS_SUBMESH_DATA &MeshData) {
  SAFE_RELEASE(MeshData.IndexBuffer);
  SAFE_RELEASE(MeshData.VertexBuffer);
}

static bool G_SpriteRectHasBuilt = false;
static RS_SUBMESH_DATA G_SpriteData = {};

RSGeometryGenerator::RSGeometryGenerator(RSRoot_DX11 *RootPtr)
    : MeshHelper(RootPtr->getMeshHelper()), Devices(RootPtr->getDevices()),
      ResourceManager(RootPtr->getResourceManager()) {}

RSGeometryGenerator::~RSGeometryGenerator() {
  SAFE_RELEASE(G_SpriteData.IndexBuffer);
  SAFE_RELEASE(G_SpriteData.VertexBuffer);
}

RS_SUBMESH_DATA
RSGeometryGenerator::createBox(float Width,
                               float Height,
                               float Depth,
                               UINT DivideNumber,
                               LAYOUT_TYPE LayoutType,
                               bool EnabledVertexColorFlag,
                               const dx::XMFLOAT4 &VertexColor,
                               const std::string &TextureName) {
  RS_SUBMESH_DATA RSD = {};
  SUBMESH_INFO SI = {};
  MATERIAL_INFO MI = {};
  std::vector<UINT> Indeices = {};
  std::vector<vertex_type::BasicVertex> BasicPtr = {};
  std::vector<vertex_type::TangentVertex> TangentPtr = {};
  std::vector<vertex_type::ColorVertex> ColorPtr = {};
  std::vector<std::string> Textures = {};
  std::string Str = "";
  float HW = 0.5f * Width;
  float HH = 0.5f * Height;
  float HD = 0.5f * Depth;
  Indeices.resize(36);

  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR:
    if (!EnabledVertexColorFlag) {
      assert(false && "not using vert color");
    }
    ColorPtr.resize(24);
    // front face
    ColorPtr[0] = {{-HW, -HH, -HD}, {0.0f, 0.0f, -1.0f}, VertexColor};
    ColorPtr[1] = {{-HW, +HH, -HD}, {0.0f, 0.0f, -1.0f}, VertexColor};
    ColorPtr[2] = {{+HW, +HH, -HD}, {0.0f, 0.0f, -1.0f}, VertexColor};
    ColorPtr[3] = {{+HW, -HH, -HD}, {0.0f, 0.0f, -1.0f}, VertexColor};
    // back face
    ColorPtr[4] = {{-HW, -HH, +HD}, {0.0f, 0.0f, 1.0f}, VertexColor};
    ColorPtr[5] = {{+HW, -HH, +HD}, {0.0f, 0.0f, 1.0f}, VertexColor};
    ColorPtr[6] = {{+HW, +HH, +HD}, {0.0f, 0.0f, 1.0f}, VertexColor};
    ColorPtr[7] = {{-HW, +HH, +HD}, {0.0f, 0.0f, 1.0f}, VertexColor};
    // top face
    ColorPtr[8] = {{-HW, +HH, -HD}, {0.0f, 1.0f, 0.0f}, VertexColor};
    ColorPtr[9] = {{-HW, +HH, +HD}, {0.0f, 1.0f, 0.0f}, VertexColor};
    ColorPtr[10] = {{+HW, +HH, +HD}, {0.0f, 1.0f, 0.0f}, VertexColor};
    ColorPtr[11] = {{+HW, +HH, -HD}, {0.0f, 1.0f, 0.0f}, VertexColor};
    // bottom face
    ColorPtr[12] = {{-HW, -HH, -HD}, {0.0f, -1.0f, 0.0f}, VertexColor};
    ColorPtr[13] = {{+HW, -HH, -HD}, {0.0f, -1.0f, 0.0f}, VertexColor};
    ColorPtr[14] = {{+HW, -HH, +HD}, {0.0f, -1.0f, 0.0f}, VertexColor};
    ColorPtr[15] = {{-HW, -HH, +HD}, {0.0f, -1.0f, 0.0f}, VertexColor};
    // left face
    ColorPtr[16] = {{-HW, -HH, +HD}, {-1.0f, 0.0f, 0.0f}, VertexColor};
    ColorPtr[17] = {{-HW, +HH, +HD}, {-1.0f, 0.0f, 0.0f}, VertexColor};
    ColorPtr[18] = {{-HW, +HH, -HD}, {-1.0f, 0.0f, 0.0f}, VertexColor};
    ColorPtr[19] = {{-HW, -HH, -HD}, {-1.0f, 0.0f, 0.0f}, VertexColor};
    // right face
    ColorPtr[20] = {{+HW, -HH, -HD}, {1.0f, 0.0f, 0.0f}, VertexColor};
    ColorPtr[21] = {{+HW, +HH, -HD}, {1.0f, 0.0f, 0.0f}, VertexColor};
    ColorPtr[22] = {{+HW, +HH, +HD}, {1.0f, 0.0f, 0.0f}, VertexColor};
    ColorPtr[23] = {{+HW, -HH, +HD}, {1.0f, 0.0f, 0.0f}, VertexColor};

    // front face index
    Indeices[0] = 0;
    Indeices[1] = 1;
    Indeices[2] = 2;
    Indeices[3] = 0;
    Indeices[4] = 2;
    Indeices[5] = 3;
    // back face index
    Indeices[6] = 4;
    Indeices[7] = 5;
    Indeices[8] = 6;
    Indeices[9] = 4;
    Indeices[10] = 6;
    Indeices[11] = 7;
    // top face index
    Indeices[12] = 8;
    Indeices[13] = 9;
    Indeices[14] = 10;
    Indeices[15] = 8;
    Indeices[16] = 10;
    Indeices[17] = 11;
    // bottom face index
    Indeices[18] = 12;
    Indeices[19] = 13;
    Indeices[20] = 14;
    Indeices[21] = 12;
    Indeices[22] = 14;
    Indeices[23] = 15;
    // left face index
    Indeices[24] = 16;
    Indeices[25] = 17;
    Indeices[26] = 18;
    Indeices[27] = 16;
    Indeices[28] = 18;
    Indeices[29] = 19;
    // right face index
    Indeices[30] = 20;
    Indeices[31] = 21;
    Indeices[32] = 22;
    Indeices[33] = 20;
    Indeices[34] = 22;
    Indeices[35] = 23;

    if (DivideNumber > 6) {
      DivideNumber = 6;
    }
    for (UINT I = 0; I < DivideNumber; I++) {
      processSubDivide(LayoutType, &ColorPtr, &Indeices);
    }

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &ColorPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;

  case LAYOUT_TYPE::NORMAL_TEX:
    if (EnabledVertexColorFlag) {
      assert(false && "using vert color");
    }
    BasicPtr.resize(24);
    // front face
    BasicPtr[0] = {{-HW, -HH, -HD}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}};
    BasicPtr[1] = {{-HW, +HH, -HD}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}};
    BasicPtr[2] = {{+HW, +HH, -HD}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}};
    BasicPtr[3] = {{+HW, -HH, -HD}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}};
    // back face
    BasicPtr[4] = {{-HW, -HH, +HD}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}};
    BasicPtr[5] = {{+HW, -HH, +HD}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}};
    BasicPtr[6] = {{+HW, +HH, +HD}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}};
    BasicPtr[7] = {{-HW, +HH, +HD}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}};
    // top face
    BasicPtr[8] = {{-HW, +HH, -HD}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}};
    BasicPtr[9] = {{-HW, +HH, +HD}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}};
    BasicPtr[10] = {{+HW, +HH, +HD}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}};
    BasicPtr[11] = {{+HW, +HH, -HD}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}};
    // bottom face
    BasicPtr[12] = {{-HW, -HH, -HD}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}};
    BasicPtr[13] = {{+HW, -HH, -HD}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}};
    BasicPtr[14] = {{+HW, -HH, +HD}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}};
    BasicPtr[15] = {{-HW, -HH, +HD}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}};
    // left face
    BasicPtr[16] = {{-HW, -HH, +HD}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}};
    BasicPtr[17] = {{-HW, +HH, +HD}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}};
    BasicPtr[18] = {{-HW, +HH, -HD}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}};
    BasicPtr[19] = {{-HW, -HH, -HD}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}};
    // right face
    BasicPtr[20] = {{+HW, -HH, -HD}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}};
    BasicPtr[21] = {{+HW, +HH, -HD}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}};
    BasicPtr[22] = {{+HW, +HH, +HD}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}};
    BasicPtr[23] = {{+HW, -HH, +HD}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}};

    // front face index
    Indeices[0] = 0;
    Indeices[1] = 1;
    Indeices[2] = 2;
    Indeices[3] = 0;
    Indeices[4] = 2;
    Indeices[5] = 3;
    // back face index
    Indeices[6] = 4;
    Indeices[7] = 5;
    Indeices[8] = 6;
    Indeices[9] = 4;
    Indeices[10] = 6;
    Indeices[11] = 7;
    // top face index
    Indeices[12] = 8;
    Indeices[13] = 9;
    Indeices[14] = 10;
    Indeices[15] = 8;
    Indeices[16] = 10;
    Indeices[17] = 11;
    // bottom face index
    Indeices[18] = 12;
    Indeices[19] = 13;
    Indeices[20] = 14;
    Indeices[21] = 12;
    Indeices[22] = 14;
    Indeices[23] = 15;
    // left face index
    Indeices[24] = 16;
    Indeices[25] = 17;
    Indeices[26] = 18;
    Indeices[27] = 16;
    Indeices[28] = 18;
    Indeices[29] = 19;
    // right face index
    Indeices[30] = 20;
    Indeices[31] = 21;
    Indeices[32] = 22;
    Indeices[33] = 20;
    Indeices[34] = 22;
    Indeices[35] = 23;

    if (DivideNumber > 6) {
      DivideNumber = 6;
    }
    for (UINT I = 0; I < DivideNumber; I++) {
      processSubDivide(LayoutType, &BasicPtr, &Indeices);
    }

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &BasicPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;

  case LAYOUT_TYPE::NORMAL_TANGENT_TEX:
    if (EnabledVertexColorFlag) {
      assert(false && "using vert color");
    }
    TangentPtr.resize(24);
    // front face
    TangentPtr[0] = {
        {-HW, -HH, -HD}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}};
    TangentPtr[1] = {
        {-HW, +HH, -HD}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}};
    TangentPtr[2] = {
        {+HW, +HH, -HD}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}};
    TangentPtr[3] = {
        {+HW, -HH, -HD}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}};
    // back face
    TangentPtr[4] = {
        {-HW, -HH, +HD}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}};
    TangentPtr[5] = {
        {+HW, -HH, +HD}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}};
    TangentPtr[6] = {
        {+HW, +HH, +HD}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}};
    TangentPtr[7] = {
        {-HW, +HH, +HD}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}};
    // top face
    TangentPtr[8] = {
        {-HW, +HH, -HD}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}};
    TangentPtr[9] = {
        {-HW, +HH, +HD}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}};
    TangentPtr[10] = {
        {+HW, +HH, +HD}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}};
    TangentPtr[11] = {
        {+HW, +HH, -HD}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}};
    // bottom face
    TangentPtr[12] = {{-HW, -HH, -HD},
                      {0.0f, -1.0f, 0.0f},
                      {-1.0f, 0.0f, 0.0f},
                      {1.0f, 1.0f}};
    TangentPtr[13] = {{+HW, -HH, -HD},
                      {0.0f, -1.0f, 0.0f},
                      {-1.0f, 0.0f, 0.0f},
                      {0.0f, 1.0f}};
    TangentPtr[14] = {{+HW, -HH, +HD},
                      {0.0f, -1.0f, 0.0f},
                      {-1.0f, 0.0f, 0.0f},
                      {0.0f, 0.0f}};
    TangentPtr[15] = {{-HW, -HH, +HD},
                      {0.0f, -1.0f, 0.0f},
                      {-1.0f, 0.0f, 0.0f},
                      {1.0f, 0.0f}};
    // left face
    TangentPtr[16] = {{-HW, -HH, +HD},
                      {-1.0f, 0.0f, 0.0f},
                      {0.0f, 0.0f, -1.0f},
                      {0.0f, 1.0f}};
    TangentPtr[17] = {{-HW, +HH, +HD},
                      {-1.0f, 0.0f, 0.0f},
                      {0.0f, 0.0f, -1.0f},
                      {0.0f, 0.0f}};
    TangentPtr[18] = {{-HW, +HH, -HD},
                      {-1.0f, 0.0f, 0.0f},
                      {0.0f, 0.0f, -1.0f},
                      {1.0f, 0.0f}};
    TangentPtr[19] = {{-HW, -HH, -HD},
                      {-1.0f, 0.0f, 0.0f},
                      {0.0f, 0.0f, -1.0f},
                      {1.0f, 1.0f}};
    // right face
    TangentPtr[20] = {
        {+HW, -HH, -HD}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}};
    TangentPtr[21] = {
        {+HW, +HH, -HD}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}};
    TangentPtr[22] = {
        {+HW, +HH, +HD}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}};
    TangentPtr[23] = {
        {+HW, -HH, +HD}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}};

    // front face index
    Indeices[0] = 0;
    Indeices[1] = 1;
    Indeices[2] = 2;
    Indeices[3] = 0;
    Indeices[4] = 2;
    Indeices[5] = 3;
    // back face index
    Indeices[6] = 4;
    Indeices[7] = 5;
    Indeices[8] = 6;
    Indeices[9] = 4;
    Indeices[10] = 6;
    Indeices[11] = 7;
    // top face index
    Indeices[12] = 8;
    Indeices[13] = 9;
    Indeices[14] = 10;
    Indeices[15] = 8;
    Indeices[16] = 10;
    Indeices[17] = 11;
    // bottom face index
    Indeices[18] = 12;
    Indeices[19] = 13;
    Indeices[20] = 14;
    Indeices[21] = 12;
    Indeices[22] = 14;
    Indeices[23] = 15;
    // left face index
    Indeices[24] = 16;
    Indeices[25] = 17;
    Indeices[26] = 18;
    Indeices[27] = 16;
    Indeices[28] = 18;
    Indeices[29] = 19;
    // right face index
    Indeices[30] = 20;
    Indeices[31] = 21;
    Indeices[32] = 22;
    Indeices[33] = 20;
    Indeices[34] = 22;
    Indeices[35] = 23;

    if (DivideNumber > 6) {
      DivideNumber = 6;
    }
    for (UINT I = 0; I < DivideNumber; I++) {
      processSubDivide(LayoutType, &TangentPtr, &Indeices);
    }

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &TangentPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;

  default:
    assert(false && "unvalid layout");
  }

  return RSD;
}

RS_SUBMESH_DATA
RSGeometryGenerator::createSphere(float Radius,
                                  UINT SliceCount,
                                  UINT StackCount,
                                  LAYOUT_TYPE LayoutType,
                                  bool EnabledVertexColorFlag,
                                  const dx::XMFLOAT4 &VertexColor,
                                  const std::string &TextureName) {
  RS_SUBMESH_DATA RSD = {};
  SUBMESH_INFO SI = {};
  MATERIAL_INFO MI = {};
  std::vector<UINT> Indeices = {};
  std::vector<vertex_type::BasicVertex> BasicPtr = {};
  std::vector<vertex_type::TangentVertex> TangentPtr = {};
  std::vector<vertex_type::ColorVertex> ColorPtr = {};
  std::vector<std::string> Textures = {};

  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR: {
    if (!EnabledVertexColorFlag) {
      assert(false && "not using vert color");
    }
    vertex_type::ColorVertex TopVertex = {
        {0.0f, +Radius, 0.0f}, {0.0f, +1.0f, 0.0f}, VertexColor};
    vertex_type::ColorVertex BottomVertex = {
        {0.0f, -Radius, 0.0f}, {0.0f, -1.0f, 0.0f}, VertexColor};

    ColorPtr.emplace_back(TopVertex);

    float PhiStep = dx::XM_PI / StackCount;
    float ThetaStep = 2.0f * dx::XM_PI / SliceCount;

    for (UINT I = 1; I <= StackCount - 1; I++) {
      float Phi = I * PhiStep;

      for (UINT J = 0; J <= SliceCount; J++) {
        float Theta = J * ThetaStep;
        vertex_type::ColorVertex V = {};

        V.Position.x = Radius * sinf(Phi) * cosf(Theta);
        V.Position.y = Radius * cosf(Phi);
        V.Position.z = Radius * sinf(Phi) * sinf(Theta);

        XMVECTOR P = XMLoadFloat3(&V.Position);
        XMStoreFloat3(&V.Normal, XMVector3Normalize(P));

        V.Color = VertexColor;

        ColorPtr.emplace_back(V);
      }
    }

    ColorPtr.emplace_back(BottomVertex);

    for (UINT I = 1; I <= SliceCount; I++) {
      Indeices.emplace_back(0);
      Indeices.emplace_back(I + 1);
      Indeices.emplace_back(I);
    }

    UINT BaseIndex = 1;
    UINT RingVertexCount = SliceCount + 1;
    for (UINT I = 0; I < StackCount - 2; I++) {
      for (UINT J = 0; J < SliceCount; J++) {
        Indeices.emplace_back(BaseIndex + I * RingVertexCount + J);
        Indeices.emplace_back(BaseIndex + I * RingVertexCount + J + 1);
        Indeices.emplace_back(BaseIndex + (I + 1) * RingVertexCount + J);

        Indeices.emplace_back(BaseIndex + (I + 1) * RingVertexCount + J);
        Indeices.emplace_back(BaseIndex + I * RingVertexCount + J + 1);
        Indeices.emplace_back(BaseIndex + (I + 1) * RingVertexCount + J + 1);
      }
    }

    UINT SouthPoleIndex = static_cast<UINT>(ColorPtr.size() - 1);
    BaseIndex = SouthPoleIndex - RingVertexCount;

    for (UINT I = 0; I < SliceCount; I++) {
      Indeices.emplace_back(SouthPoleIndex);
      Indeices.emplace_back(BaseIndex + I);
      Indeices.emplace_back(BaseIndex + I + 1);
    }

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &ColorPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  case LAYOUT_TYPE::NORMAL_TEX: {
    if (EnabledVertexColorFlag) {
      assert(false && "using vert color");
    }
    vertex_type::BasicVertex TopVertex = {
        {0.0f, +Radius, 0.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 0.0f}};
    vertex_type::BasicVertex BottomVertex = {
        {0.0f, -Radius, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}};

    BasicPtr.emplace_back(TopVertex);

    float PhiStep = dx::XM_PI / StackCount;
    float ThetaStep = 2.0f * dx::XM_PI / SliceCount;

    for (UINT I = 1; I <= StackCount - 1; I++) {
      float Phi = I * PhiStep;

      for (UINT J = 0; J <= SliceCount; J++) {
        float Theta = J * ThetaStep;
        vertex_type::BasicVertex V = {};

        V.Position.x = Radius * sinf(Phi) * cosf(Theta);
        V.Position.y = Radius * cosf(Phi);
        V.Position.z = Radius * sinf(Phi) * sinf(Theta);

        XMVECTOR P = XMLoadFloat3(&V.Position);
        XMStoreFloat3(&V.Normal, XMVector3Normalize(P));

        V.TexCoord.x = Theta / dx::XM_2PI;
        V.TexCoord.y = Phi / dx::XM_PI;

        BasicPtr.emplace_back(V);
      }
    }

    BasicPtr.emplace_back(BottomVertex);

    for (UINT I = 1; I <= SliceCount; I++) {
      Indeices.emplace_back(0);
      Indeices.emplace_back(I + 1);
      Indeices.emplace_back(I);
    }

    UINT BaseIndex = 1;
    UINT RingVertexCount = SliceCount + 1;
    for (UINT I = 0; I < StackCount - 2; I++) {
      for (UINT J = 0; J < SliceCount; J++) {
        Indeices.emplace_back(BaseIndex + I * RingVertexCount + J);
        Indeices.emplace_back(BaseIndex + I * RingVertexCount + J + 1);
        Indeices.emplace_back(BaseIndex + (I + 1) * RingVertexCount + J);

        Indeices.emplace_back(BaseIndex + (I + 1) * RingVertexCount + J);
        Indeices.emplace_back(BaseIndex + I * RingVertexCount + J + 1);
        Indeices.emplace_back(BaseIndex + (I + 1) * RingVertexCount + J + 1);
      }
    }

    UINT SouthPoleIndex = (UINT)BasicPtr.size() - 1;
    BaseIndex = SouthPoleIndex - RingVertexCount;

    for (UINT I = 0; I < SliceCount; I++) {
      Indeices.emplace_back(SouthPoleIndex);
      Indeices.emplace_back(BaseIndex + I);
      Indeices.emplace_back(BaseIndex + I + 1);
    }

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &BasicPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  case LAYOUT_TYPE::NORMAL_TANGENT_TEX: {
    if (EnabledVertexColorFlag) {
      assert(false && "using vert color");
    }
    vertex_type::TangentVertex TopVertex = {{0.0f, +Radius, 0.0f},
                                            {0.0f, +1.0f, 0.0f},
                                            {1.0f, 0.0f, 0.0f},
                                            {0.0f, 0.0f}};
    vertex_type::TangentVertex BottomVertex = {{0.0f, -Radius, 0.0f},
                                               {0.0f, -1.0f, 0.0f},
                                               {1.0f, 0.0f, 0.0f},
                                               {0.0f, 1.0f}};

    TangentPtr.emplace_back(TopVertex);

    float PhiStep = dx::XM_PI / StackCount;
    float ThetaStep = 2.0f * dx::XM_PI / SliceCount;

    for (UINT I = 1; I <= StackCount - 1; I++) {
      float Phi = I * PhiStep;

      for (UINT J = 0; J <= SliceCount; J++) {
        float Theta = J * ThetaStep;
        vertex_type::TangentVertex V = {};

        V.Position.x = Radius * sinf(Phi) * cosf(Theta);
        V.Position.y = Radius * cosf(Phi);
        V.Position.z = Radius * sinf(Phi) * sinf(Theta);
        V.Tangent.x = -Radius * sinf(Phi) * sinf(Theta);
        V.Tangent.y = 0.0f;
        V.Tangent.z = +Radius * sinf(Phi) * cosf(Theta);

        XMVECTOR T = XMLoadFloat3(&V.Tangent);
        XMStoreFloat3(&V.Tangent, XMVector3Normalize(T));
        XMVECTOR P = XMLoadFloat3(&V.Position);
        XMStoreFloat3(&V.Normal, XMVector3Normalize(P));

        V.TexCoord.x = Theta / dx::XM_2PI;
        V.TexCoord.y = Phi / dx::XM_PI;

        TangentPtr.emplace_back(V);
      }
    }

    TangentPtr.emplace_back(BottomVertex);

    for (UINT I = 1; I <= SliceCount; I++) {
      Indeices.emplace_back(0);
      Indeices.emplace_back(I + 1);
      Indeices.emplace_back(I);
    }

    UINT BaseIndex = 1;
    UINT RingVertexCount = SliceCount + 1;
    for (UINT I = 0; I < StackCount - 2; I++) {
      for (UINT J = 0; J < SliceCount; J++) {
        Indeices.emplace_back(BaseIndex + I * RingVertexCount + J);
        Indeices.emplace_back(BaseIndex + I * RingVertexCount + J + 1);
        Indeices.emplace_back(BaseIndex + (I + 1) * RingVertexCount + J);

        Indeices.emplace_back(BaseIndex + (I + 1) * RingVertexCount + J);
        Indeices.emplace_back(BaseIndex + I * RingVertexCount + J + 1);
        Indeices.emplace_back(BaseIndex + (I + 1) * RingVertexCount + J + 1);
      }
    }

    UINT SouthPoleIndex = static_cast<UINT>(TangentPtr.size() - 1);
    BaseIndex = SouthPoleIndex - RingVertexCount;

    for (UINT I = 0; I < SliceCount; I++) {
      Indeices.emplace_back(SouthPoleIndex);
      Indeices.emplace_back(BaseIndex + I);
      Indeices.emplace_back(BaseIndex + I + 1);
    }

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &TangentPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  default:
    assert(false && "unvalid layout");
  }

  return RSD;
}

RS_SUBMESH_DATA
RSGeometryGenerator::createGeometrySphere(float Radius,
                                          UINT DivideNumber,
                                          LAYOUT_TYPE LayoutType,
                                          bool EnabledVertexColorFlag,
                                          const dx::XMFLOAT4 &VertexColor,
                                          const std::string &TextureName) {
  RS_SUBMESH_DATA RSD = {};
  SUBMESH_INFO SI = {};
  MATERIAL_INFO MI = {};
  std::vector<UINT> Indeices = {};
  std::vector<vertex_type::BasicVertex> BasicPtr = {};
  std::vector<vertex_type::TangentVertex> TangentPtr = {};
  std::vector<vertex_type::ColorVertex> ColorPtr = {};
  std::vector<std::string> Textures = {};

  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR: {
    if (!EnabledVertexColorFlag) {
      assert(false && "not using vert color");
    }

    if (DivideNumber > 6) {
      DivideNumber = 6;
    }

    const float X = 0.525731f;
    const float Z = 0.850651f;
    XMFLOAT3 Pos[12] = {
        XMFLOAT3(-X, 0.0f, Z), XMFLOAT3(X, 0.0f, Z),   XMFLOAT3(-X, 0.0f, -Z),
        XMFLOAT3(X, 0.0f, -Z), XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
        XMFLOAT3(0.0f, -Z, X), XMFLOAT3(0.0f, -Z, -X), XMFLOAT3(Z, X, 0.0f),
        XMFLOAT3(-Z, X, 0.0f), XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)};
    UINT K[60] = {1,  4,  0, 4,  9, 0, 4, 5,  9, 8, 5, 4,  1,  8, 4,
                  1,  10, 8, 10, 3, 8, 8, 3,  5, 3, 2, 5,  3,  7, 2,
                  3,  10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6,  1, 0,
                  10, 1,  6, 11, 0, 9, 2, 11, 9, 5, 2, 9,  11, 2, 7};

    ColorPtr.resize(ARRAYSIZE(Pos));
    Indeices.assign(&K[0], &K[60]);

    for (UINT I = 0, E = ARRAYSIZE(Pos); I < E; I++) {
      ColorPtr[I].Position = Pos[I];
    }
    for (UINT I = 0; I < DivideNumber; I++) {
      processSubDivide(LayoutType, &ColorPtr, &Indeices);
    }

    for (UINT I = 0, E = static_cast<UINT>(ColorPtr.size()); I < E; ++I) {
      XMVECTOR N = XMVector3Normalize(XMLoadFloat3(&ColorPtr[I].Position));
      XMVECTOR P = Radius * N;

      XMStoreFloat3(&ColorPtr[I].Position, P);
      XMStoreFloat3(&ColorPtr[I].Normal, N);

      float Theta = atan2f(ColorPtr[I].Position.z, ColorPtr[I].Position.x);

      if (Theta < 0.0f) {
        Theta += dx::XM_2PI;
      }

      float Phi = acosf(ColorPtr[I].Position.y / Radius);
      ColorPtr[I].Color = VertexColor;
      (void)Phi;
      (void)Theta;
    }

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &ColorPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  case LAYOUT_TYPE::NORMAL_TEX: {
    if (EnabledVertexColorFlag) {
      assert(false && "using vert color");
    }

    if (DivideNumber > 6) {
      DivideNumber = 6;
    }

    const float X = 0.525731f;
    const float Z = 0.850651f;
    XMFLOAT3 Pos[12] = {
        XMFLOAT3(-X, 0.0f, Z), XMFLOAT3(X, 0.0f, Z),   XMFLOAT3(-X, 0.0f, -Z),
        XMFLOAT3(X, 0.0f, -Z), XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
        XMFLOAT3(0.0f, -Z, X), XMFLOAT3(0.0f, -Z, -X), XMFLOAT3(Z, X, 0.0f),
        XMFLOAT3(-Z, X, 0.0f), XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)};
    UINT K[60] = {1,  4,  0, 4,  9, 0, 4, 5,  9, 8, 5, 4,  1,  8, 4,
                  1,  10, 8, 10, 3, 8, 8, 3,  5, 3, 2, 5,  3,  7, 2,
                  3,  10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6,  1, 0,
                  10, 1,  6, 11, 0, 9, 2, 11, 9, 5, 2, 9,  11, 2, 7};

    BasicPtr.resize(ARRAYSIZE(Pos));
    Indeices.assign(&K[0], &K[60]);

    for (UINT I = 0, E = ARRAYSIZE(Pos); I < E; I++) {
      BasicPtr[I].Position = Pos[I];
    }
    for (UINT I = 0; I < DivideNumber; I++) {
      processSubDivide(LayoutType, &BasicPtr, &Indeices);
    }

    for (UINT I = 0; I < (UINT)BasicPtr.size(); ++I) {
      XMVECTOR N = XMVector3Normalize(XMLoadFloat3(&BasicPtr[I].Position));
      XMVECTOR P = Radius * N;

      XMStoreFloat3(&BasicPtr[I].Position, P);
      XMStoreFloat3(&BasicPtr[I].Normal, N);

      float Theta = atan2f(BasicPtr[I].Position.z, BasicPtr[I].Position.x);

      if (Theta < 0.0f) {
        Theta += dx::XM_2PI;
      }

      float Phi = acosf(BasicPtr[I].Position.y / Radius);

      BasicPtr[I].TexCoord.x = Theta / dx::XM_2PI;
      BasicPtr[I].TexCoord.y = Phi / dx::XM_PI;
    }

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &BasicPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  case LAYOUT_TYPE::NORMAL_TANGENT_TEX: {
    if (EnabledVertexColorFlag) {
      assert(false && "using vert color");
    }

    if (DivideNumber > 6) {
      DivideNumber = 6;
    }

    const float X = 0.525731f;
    const float Z = 0.850651f;
    XMFLOAT3 Pos[12] = {
        XMFLOAT3(-X, 0.0f, Z), XMFLOAT3(X, 0.0f, Z),   XMFLOAT3(-X, 0.0f, -Z),
        XMFLOAT3(X, 0.0f, -Z), XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
        XMFLOAT3(0.0f, -Z, X), XMFLOAT3(0.0f, -Z, -X), XMFLOAT3(Z, X, 0.0f),
        XMFLOAT3(-Z, X, 0.0f), XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)};
    UINT K[60] = {1,  4,  0, 4,  9, 0, 4, 5,  9, 8, 5, 4,  1,  8, 4,
                  1,  10, 8, 10, 3, 8, 8, 3,  5, 3, 2, 5,  3,  7, 2,
                  3,  10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6,  1, 0,
                  10, 1,  6, 11, 0, 9, 2, 11, 9, 5, 2, 9,  11, 2, 7};

    TangentPtr.resize(ARRAYSIZE(Pos));
    Indeices.assign(&K[0], &K[60]);

    for (UINT I = 0, E = ARRAYSIZE(Pos); I < E; I++) {
      TangentPtr[I].Position = Pos[I];
    }
    for (UINT I = 0; I < DivideNumber; I++) {
      processSubDivide(LayoutType, &TangentPtr, &Indeices);
    }

    for (UINT I = 0, E = static_cast<UINT>(TangentPtr.size()); I < E; I++) {
      XMVECTOR N = XMVector3Normalize(XMLoadFloat3(&TangentPtr[I].Position));
      XMVECTOR P = Radius * N;

      XMStoreFloat3(&TangentPtr[I].Position, P);
      XMStoreFloat3(&TangentPtr[I].Normal, N);

      float Theta = atan2f(TangentPtr[I].Position.z, TangentPtr[I].Position.x);

      if (Theta < 0.0f) {
        Theta += dx::XM_2PI;
      }

      float Phi = acosf(TangentPtr[I].Position.y / Radius);

      TangentPtr[I].TexCoord.x = Theta / dx::XM_2PI;
      TangentPtr[I].TexCoord.y = Phi / dx::XM_PI;

      TangentPtr[I].Tangent.x = -Radius * sinf(Phi) * sinf(Theta);
      TangentPtr[I].Tangent.y = 0.0f;
      TangentPtr[I].Tangent.z = +Radius * sinf(Phi) * cosf(Theta);

      XMVECTOR T = XMLoadFloat3(&TangentPtr[I].Tangent);
      XMStoreFloat3(&TangentPtr[I].Tangent, XMVector3Normalize(T));
    }

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &TangentPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  default:
    assert(false && "unvalid layout");
  }

  return RSD;
}

RS_SUBMESH_DATA
RSGeometryGenerator::createCylinder(float BottomRadius,
                                    float TopRadius,
                                    float Height,
                                    UINT SliceCount,
                                    UINT StackCount,
                                    LAYOUT_TYPE LayoutType,
                                    bool EnabledVertexColorFlag,
                                    const dx::XMFLOAT4 &VertexColor,
                                    const std::string &TextureName) {
  RS_SUBMESH_DATA RSD = {};
  SUBMESH_INFO SI = {};
  MATERIAL_INFO MI = {};
  std::vector<UINT> Indeices = {};
  std::vector<vertex_type::BasicVertex> BasicPtr = {};
  std::vector<vertex_type::TangentVertex> TangentPtr = {};
  std::vector<vertex_type::ColorVertex> ColorPtr = {};
  std::vector<std::string> Textures = {};

  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR: {
    float StackHeight = Height / StackCount;
    float RadiusStep = (TopRadius - BottomRadius) / StackCount;
    UINT RingCount = StackCount + 1;

    for (UINT I = 0; I < RingCount; I++) {
      float Y = -0.5f * Height + I * StackHeight;
      float R = BottomRadius + I * RadiusStep;
      float DTheta = 2.0f * dx::XM_PI / SliceCount;

      for (UINT J = 0; J <= SliceCount; J++) {
        vertex_type::ColorVertex Vertex = {};

        float C = cosf(J * DTheta);
        float S = sinf(J * DTheta);

        Vertex.Color = VertexColor;
        Vertex.Position = dx::XMFLOAT3(R * C, Y, R * S);
        dx::XMFLOAT3 TangentV = dx::XMFLOAT3(-S, 0.f, C);

        float DR = BottomRadius - TopRadius;
        XMFLOAT3 Bitangent(DR * C, -Height, DR * S);
        XMVECTOR T = dx::XMLoadFloat3(&TangentV);
        XMVECTOR B = dx::XMLoadFloat3(&Bitangent);
        XMVECTOR N = dx::XMVector3Normalize(dx::XMVector3Cross(T, B));
        dx::XMStoreFloat3(&Vertex.Normal, N);

        ColorPtr.emplace_back(Vertex);
      }
    }

    UINT RingVertexCount = SliceCount + 1;

    for (UINT I = 0; I < StackCount; ++I) {
      for (UINT J = 0; J < SliceCount; ++J) {
        Indeices.emplace_back(I * RingVertexCount + J);
        Indeices.emplace_back((I + 1) * RingVertexCount + J);
        Indeices.emplace_back((I + 1) * RingVertexCount + J + 1);

        Indeices.emplace_back(I * RingVertexCount + J);
        Indeices.emplace_back((I + 1) * RingVertexCount + J + 1);
        Indeices.emplace_back(I * RingVertexCount + J + 1);
      }
    }

    buildCylinderTopCap(BottomRadius, TopRadius, Height, SliceCount, StackCount,
                        LayoutType, &ColorPtr, &Indeices, VertexColor);
    buildCylinderBottomCap(BottomRadius, TopRadius, Height, SliceCount,
                           StackCount, LayoutType, &ColorPtr, &Indeices,
                           VertexColor);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &ColorPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  case LAYOUT_TYPE::NORMAL_TEX: {
    float StackHeight = Height / StackCount;
    float RadiusStep = (TopRadius - BottomRadius) / StackCount;
    UINT RingCount = StackCount + 1;

    for (UINT I = 0; I < RingCount; I++) {
      float Y = -0.5f * Height + I * StackHeight;
      float R = BottomRadius + I * RadiusStep;
      float DTheta = 2.0f * dx::XM_PI / SliceCount;

      for (UINT J = 0; J <= SliceCount; J++) {
        vertex_type::BasicVertex Vertex = {};

        float C = cosf(J * DTheta);
        float S = sinf(J * DTheta);

        Vertex.Position = dx::XMFLOAT3(R * C, Y, R * S);
        Vertex.TexCoord.x = (float)J / SliceCount;
        Vertex.TexCoord.y = 1.0f - (float)I / StackCount;
        dx::XMFLOAT3 TangentV = dx::XMFLOAT3(-S, 0.f, C);

        float DR = BottomRadius - TopRadius;
        XMFLOAT3 Bitangent(DR * C, -Height, DR * S);
        XMVECTOR T = dx::XMLoadFloat3(&TangentV);
        XMVECTOR B = dx::XMLoadFloat3(&Bitangent);
        XMVECTOR N = dx::XMVector3Normalize(dx::XMVector3Cross(T, B));
        dx::XMStoreFloat3(&Vertex.Normal, N);

        BasicPtr.emplace_back(Vertex);
      }
    }

    UINT RingVertexCount = SliceCount + 1;

    for (UINT I = 0; I < StackCount; ++I) {
      for (UINT J = 0; J < SliceCount; ++J) {
        Indeices.emplace_back(I * RingVertexCount + J);
        Indeices.emplace_back((I + 1) * RingVertexCount + J);
        Indeices.emplace_back((I + 1) * RingVertexCount + J + 1);

        Indeices.emplace_back(I * RingVertexCount + J);
        Indeices.emplace_back((I + 1) * RingVertexCount + J + 1);
        Indeices.emplace_back(I * RingVertexCount + J + 1);
      }
    }

    buildCylinderTopCap(BottomRadius, TopRadius, Height, SliceCount, StackCount,
                        LayoutType, &BasicPtr, &Indeices, VertexColor);
    buildCylinderBottomCap(BottomRadius, TopRadius, Height, SliceCount,
                           StackCount, LayoutType, &BasicPtr, &Indeices,
                           VertexColor);

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &BasicPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  case LAYOUT_TYPE::NORMAL_TANGENT_TEX: {
    float StackHeight = Height / StackCount;
    float RadiusStep = (TopRadius - BottomRadius) / StackCount;
    UINT RingCount = StackCount + 1;

    for (UINT I = 0; I < RingCount; I++) {
      float Y = -0.5f * Height + I * StackHeight;
      float R = BottomRadius + I * RadiusStep;
      float DTheta = 2.0f * dx::XM_PI / SliceCount;

      for (UINT J = 0; J <= SliceCount; J++) {
        vertex_type::TangentVertex Vertex = {};

        float C = cosf(J * DTheta);
        float S = sinf(J * DTheta);

        Vertex.Position = dx::XMFLOAT3(R * C, Y, R * S);
        Vertex.TexCoord.x = (float)J / SliceCount;
        Vertex.TexCoord.y = 1.0f - (float)I / StackCount;
        Vertex.Tangent = dx::XMFLOAT3(-S, 0.f, C);

        float DR = BottomRadius - TopRadius;
        XMFLOAT3 Bitangent(DR * C, -Height, DR * S);
        XMVECTOR T = dx::XMLoadFloat3(&Vertex.Tangent);
        XMVECTOR B = dx::XMLoadFloat3(&Bitangent);
        XMVECTOR N = dx::XMVector3Normalize(dx::XMVector3Cross(T, B));
        dx::XMStoreFloat3(&Vertex.Normal, N);

        TangentPtr.emplace_back(Vertex);
      }
    }

    UINT RingVertexCount = SliceCount + 1;

    for (UINT I = 0; I < StackCount; ++I) {
      for (UINT J = 0; J < SliceCount; ++J) {
        Indeices.emplace_back(I * RingVertexCount + J);
        Indeices.emplace_back((I + 1) * RingVertexCount + J);
        Indeices.emplace_back((I + 1) * RingVertexCount + J + 1);

        Indeices.emplace_back(I * RingVertexCount + J);
        Indeices.emplace_back((I + 1) * RingVertexCount + J + 1);
        Indeices.emplace_back(I * RingVertexCount + J + 1);
      }
    }

    buildCylinderTopCap(BottomRadius, TopRadius, Height, SliceCount, StackCount,
                        LayoutType, &TangentPtr, &Indeices, VertexColor);
    buildCylinderBottomCap(BottomRadius, TopRadius, Height, SliceCount,
                           StackCount, LayoutType, &TangentPtr, &Indeices,
                           VertexColor);

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &TangentPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  default:
    assert(false && "null layout");
  }

  return RSD;
}

RS_SUBMESH_DATA
RSGeometryGenerator::createGrid(float Width,
                                float Depth,
                                UINT RowCount,
                                UINT ColCount,
                                LAYOUT_TYPE LayoutType,
                                bool EnabledVertexColorFlag,
                                const dx::XMFLOAT4 &VertexColor,
                                const std::string &TextureName) {
  RS_SUBMESH_DATA RSD = {};
  SUBMESH_INFO SI = {};
  MATERIAL_INFO MI = {};
  std::vector<UINT> Indeices = {};
  std::vector<vertex_type::BasicVertex> BasicPtr = {};
  std::vector<vertex_type::TangentVertex> TangentPtr = {};
  std::vector<vertex_type::ColorVertex> ColorPtr = {};
  std::vector<std::string> Textures = {};

  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR: {
    UINT VertexCount = RowCount * ColCount;
    UINT FaceCount = (RowCount - 1) * (ColCount - 1) * 2;
    float HalfWidth = 0.5f * Width;
    float HalfDepth = 0.5f * Depth;
    float DX = Width / (ColCount - 1);
    float DZ = Depth / (RowCount - 1);
    float DU = 1.f / (ColCount - 1);
    float DV = 1.f / (RowCount - 1);
    (void)DU;
    (void)DV;

    ColorPtr.resize(VertexCount);
    for (UINT I = 0; I < RowCount; I++) {
      float Z = HalfDepth - I * DZ;
      for (UINT J = 0; J < ColCount; J++) {
        float X = -HalfWidth + J * DX;

        ColorPtr[static_cast<std::vector<
                     vertex_type::ColorVertex,
                     std::allocator<vertex_type::ColorVertex>>::size_type>(I) *
                     ColCount +
                 J]
            .Position = dx::XMFLOAT3(X, 0.f, Z);
        ColorPtr[static_cast<std::vector<
                     vertex_type::ColorVertex,
                     std::allocator<vertex_type::ColorVertex>>::size_type>(I) *
                     ColCount +
                 J]
            .Normal = dx::XMFLOAT3(0.0f, 1.0f, 0.0f);
        ColorPtr[static_cast<std::vector<
                     vertex_type::ColorVertex,
                     std::allocator<vertex_type::ColorVertex>>::size_type>(I) *
                     ColCount +
                 J]
            .Color = VertexColor;
      }
    }

    Indeices.resize(
        static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(
            FaceCount) *
        3);

    UINT K = 0;
    for (UINT I = 0; I < RowCount - 1; I++) {
      for (UINT J = 0; J < ColCount - 1; J++) {
        Indeices[static_cast<
            std::vector<UINT, std::allocator<UINT>>::size_type>(K)] =
            I * ColCount + J;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 1] = I * ColCount + J + 1;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 2] = (I + 1) * ColCount + J;

        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 3] = (I + 1) * ColCount + J;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 4] = I * ColCount + J + 1;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 5] = (I + 1) * ColCount + J + 1;

        K += 6;
      }
    }

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &ColorPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  case LAYOUT_TYPE::NORMAL_TEX: {
    UINT VertexCount = RowCount * ColCount;
    UINT FaceCount = (RowCount - 1) * (ColCount - 1) * 2;
    float HalfWidth = 0.5f * Width;
    float HalfDepth = 0.5f * Depth;
    float DX = Width / (ColCount - 1);
    float DZ = Depth / (RowCount - 1);
    float DU = 1.f / (ColCount - 1);
    float DV = 1.f / (RowCount - 1);

    BasicPtr.resize(VertexCount);
    for (UINT I = 0; I < RowCount; I++) {
      float Z = HalfDepth - I * DZ;
      for (UINT J = 0; J < ColCount; J++) {
        float X = -HalfWidth + J * DX;

        BasicPtr[static_cast<std::vector<
                     vertex_type::BasicVertex,
                     std::allocator<vertex_type::BasicVertex>>::size_type>(I) *
                     ColCount +
                 J]
            .Position = dx::XMFLOAT3(X, 0.f, Z);
        BasicPtr[static_cast<std::vector<
                     vertex_type::BasicVertex,
                     std::allocator<vertex_type::BasicVertex>>::size_type>(I) *
                     ColCount +
                 J]
            .Normal = dx::XMFLOAT3(0.0f, 1.0f, 0.0f);
        BasicPtr[static_cast<std::vector<
                     vertex_type::BasicVertex,
                     std::allocator<vertex_type::BasicVertex>>::size_type>(I) *
                     ColCount +
                 J]
            .TexCoord.x = J * DU;
        BasicPtr[static_cast<std::vector<
                     vertex_type::BasicVertex,
                     std::allocator<vertex_type::BasicVertex>>::size_type>(I) *
                     ColCount +
                 J]
            .TexCoord.y = I * DV;
      }
    }

    Indeices.resize(
        static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(
            FaceCount) *
        3);

    UINT K = 0;
    for (UINT I = 0; I < RowCount - 1; I++) {
      for (UINT J = 0; J < ColCount - 1; J++) {
        Indeices[static_cast<
            std::vector<UINT, std::allocator<UINT>>::size_type>(K)] =
            I * ColCount + J;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 1] = I * ColCount + J + 1;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 2] = (I + 1) * ColCount + J;

        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 3] = (I + 1) * ColCount + J;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 4] = I * ColCount + J + 1;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 5] = (I + 1) * ColCount + J + 1;

        K += 6;
      }
    }

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &BasicPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  case LAYOUT_TYPE::NORMAL_TANGENT_TEX: {
    UINT VertexCount = RowCount * ColCount;
    UINT FaceCount = (RowCount - 1) * (ColCount - 1) * 2;
    float HalfWidth = 0.5f * Width;
    float HalfDepth = 0.5f * Depth;
    float DX = Width / (ColCount - 1);
    float DZ = Depth / (RowCount - 1);
    float DU = 1.f / (ColCount - 1);
    float DV = 1.f / (RowCount - 1);

    TangentPtr.resize(VertexCount);
    for (UINT I = 0; I < RowCount; I++) {
      float Z = HalfDepth - I * DZ;
      for (UINT J = 0; J < ColCount; J++) {
        float X = -HalfWidth + J * DX;

        TangentPtr[static_cast<std::vector<
                       vertex_type::TangentVertex,
                       std::allocator<vertex_type::TangentVertex>>::size_type>(
                       I) *
                       ColCount +
                   J]
            .Position = dx::XMFLOAT3(X, 0.f, Z);
        TangentPtr[static_cast<std::vector<
                       vertex_type::TangentVertex,
                       std::allocator<vertex_type::TangentVertex>>::size_type>(
                       I) *
                       ColCount +
                   J]
            .Normal = dx::XMFLOAT3(0.0f, 1.0f, 0.0f);
        TangentPtr[static_cast<std::vector<
                       vertex_type::TangentVertex,
                       std::allocator<vertex_type::TangentVertex>>::size_type>(
                       I) *
                       ColCount +
                   J]
            .Tangent = dx::XMFLOAT3(1.0f, 0.0f, 0.0f);

        TangentPtr[static_cast<std::vector<
                       vertex_type::TangentVertex,
                       std::allocator<vertex_type::TangentVertex>>::size_type>(
                       I) *
                       ColCount +
                   J]
            .TexCoord.x = J * DU;
        TangentPtr[static_cast<std::vector<
                       vertex_type::TangentVertex,
                       std::allocator<vertex_type::TangentVertex>>::size_type>(
                       I) *
                       ColCount +
                   J]
            .TexCoord.y = I * DV;
      }
    }

    Indeices.resize(
        static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(
            FaceCount) *
        3);

    UINT K = 0;
    for (UINT I = 0; I < RowCount - 1; I++) {
      for (UINT J = 0; J < ColCount - 1; J++) {
        Indeices[static_cast<
            std::vector<UINT, std::allocator<UINT>>::size_type>(K)] =
            I * ColCount + J;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 1] = I * ColCount + J + 1;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 2] = (I + 1) * ColCount + J;

        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 3] = (I + 1) * ColCount + J;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 4] = I * ColCount + J + 1;
        Indeices[static_cast<
                     std::vector<UINT, std::allocator<UINT>>::size_type>(K) +
                 5] = (I + 1) * ColCount + J + 1;

        K += 6;
      }
    }

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.VerteicesPtr = &TangentPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;
  }

  default:
    assert(false && "nullLayout");
  }

  return RSD;
}

RS_SUBMESH_DATA
RSGeometryGenerator::createSpriteRect(LAYOUT_TYPE LayoutType,
                                      const std::string &TexturePath) {
  static SUBMESH_INFO SI = {};
  static MATERIAL_INFO MI = {};
  static std::vector<UINT> Indeices = {};
  static std::vector<vertex_type::BasicVertex> BasicPtr = {};
  static std::vector<vertex_type::TangentVertex> TangentPtr = {};
  static std::vector<std::string> Textures = {};

  if (!G_SpriteRectHasBuilt) {
    BasicPtr.resize(4);
    TangentPtr.resize(4);
    Indeices.resize(6);

    BasicPtr[0].Position = {-0.5f, -0.5f, 0.f};
    BasicPtr[0].Normal = {0.0f, 0.0f, -1.0f};
    BasicPtr[0].TexCoord = {0.0f, 1.0f};
    BasicPtr[1].Position = {-0.5f, +0.5f, 0.f};
    BasicPtr[1].Normal = {0.0f, 0.0f, -1.0f};
    BasicPtr[1].TexCoord = {0.0f, 0.0f};
    BasicPtr[2].Position = {+0.5f, +0.5f, 0.f};
    BasicPtr[2].Normal = {0.0f, 0.0f, -1.0f};
    BasicPtr[2].TexCoord = {1.0f, 0.0f};
    BasicPtr[3].Position = {+0.5f, -0.5f, 0.f};
    BasicPtr[3].Normal = {0.0f, 0.0f, -1.0f};
    BasicPtr[3].TexCoord = {1.0f, 1.0f};
    TangentPtr[0].Position = BasicPtr[0].Position;
    TangentPtr[0].Normal = BasicPtr[0].Normal;
    TangentPtr[0].Tangent = {1.0f, 0.0f, 0.0f};
    TangentPtr[0].TexCoord = BasicPtr[0].TexCoord;
    TangentPtr[1].Position = BasicPtr[1].Position;
    TangentPtr[1].Normal = BasicPtr[1].Normal;
    TangentPtr[1].Tangent = {1.0f, 0.0f, 0.0f};
    TangentPtr[1].TexCoord = BasicPtr[1].TexCoord;
    TangentPtr[2].Position = BasicPtr[2].Position;
    TangentPtr[2].Normal = BasicPtr[2].Normal;
    TangentPtr[2].Tangent = {1.0f, 0.0f, 0.0f};
    TangentPtr[2].TexCoord = BasicPtr[2].TexCoord;
    TangentPtr[3].Position = BasicPtr[3].Position;
    TangentPtr[3].Normal = BasicPtr[3].Normal;
    TangentPtr[3].Tangent = {1.0f, 0.0f, 0.0f};
    TangentPtr[3].TexCoord = BasicPtr[3].TexCoord;
    Indeices[0] = 0;
    Indeices[1] = 1;
    Indeices[2] = 2;
    Indeices[3] = 0;
    Indeices[4] = 2;
    Indeices[5] = 3;

    SI.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
    SI.IndeicesPtr = &Indeices;
    switch (LayoutType) {
    case LAYOUT_TYPE::NORMAL_TEX:
      SI.VerteicesPtr = &BasicPtr;
      break;
    case LAYOUT_TYPE::NORMAL_TANGENT_TEX:
      SI.VerteicesPtr = &TangentPtr;
      break;
    default:
      assert(false && "unvalid layout");
      break;
    }
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&G_SpriteData, &SI, LayoutType);
    G_SpriteRectHasBuilt = true;
  }

  static std::wstring WStr = L"";
  static std::string Name = "";
  static HRESULT Hr = S_OK;
  ID3D11ShaderResourceView *Srv = nullptr;
  WStr = std::wstring(TexturePath.begin(), TexturePath.end());
  WStr = L".\\Assets\\Textures\\" + WStr;
  if (TexturePath.find(".dds") != std::string::npos ||
      TexturePath.find(".DDS") != std::string::npos) {
    Hr = dx::CreateDDSTextureFromFile(Devices->getDevice(), WStr.c_str(),
                                      nullptr, &Srv);
    if (SUCCEEDED(Hr)) {
      Name = TexturePath;
      ResourceManager->addMeshSrv(Name, Srv);
      G_SpriteData.Textures[0] = Name;
    } else {
      assert(false && "texture load fail");
    }
  } else {
    Hr = dx::CreateWICTextureFromFile(Devices->getDevice(), WStr.c_str(),
                                      nullptr, &Srv);
    if (SUCCEEDED(Hr)) {
      Name = TexturePath;
      ResourceManager->addMeshSrv(Name, Srv);
      G_SpriteData.Textures[0] = Name;
    } else {
      assert(false && "texture load fail");
    }
  }

  return G_SpriteData;
}

void
RSGeometryGenerator::processSubDivide(LAYOUT_TYPE LayoutType,
                                      void *RawVertexArray,
                                      std::vector<UINT> *RawIndexArray) {
  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR: {
    std::vector<vertex_type::ColorVertex> *ColorArray =
        static_cast<std::vector<vertex_type::ColorVertex> *>(RawVertexArray);
    std::vector<vertex_type::ColorVertex> ColorCopy = *ColorArray;
    std::vector<UINT> IndexCopy = *RawIndexArray;

    ColorArray->resize(0);
    RawIndexArray->resize(0);

    UINT NumTris = static_cast<UINT>(IndexCopy.size()) / 3;
    for (UINT I = 0; I < NumTris; I++) {
      vertex_type::ColorVertex V0 = ColorCopy[IndexCopy[I * 3 + 0]];
      vertex_type::ColorVertex V1 = ColorCopy[IndexCopy[I * 3 + 1]];
      vertex_type::ColorVertex V2 = ColorCopy[IndexCopy[I * 3 + 2]];

      vertex_type::ColorVertex M0 = createColorMidPoint(V0, V1);
      vertex_type::ColorVertex M1 = createColorMidPoint(V1, V2);
      vertex_type::ColorVertex M2 = createColorMidPoint(V0, V2);

      ColorArray->emplace_back(V0);
      ColorArray->emplace_back(V1);
      ColorArray->emplace_back(V2);
      ColorArray->emplace_back(M0);
      ColorArray->emplace_back(M1);
      ColorArray->emplace_back(M2);
      RawIndexArray->emplace_back(I * 6 + 0);
      RawIndexArray->emplace_back(I * 6 + 3);
      RawIndexArray->emplace_back(I * 6 + 5);
      RawIndexArray->emplace_back(I * 6 + 3);
      RawIndexArray->emplace_back(I * 6 + 4);
      RawIndexArray->emplace_back(I * 6 + 5);
      RawIndexArray->emplace_back(I * 6 + 5);
      RawIndexArray->emplace_back(I * 6 + 4);
      RawIndexArray->emplace_back(I * 6 + 2);
      RawIndexArray->emplace_back(I * 6 + 3);
      RawIndexArray->emplace_back(I * 6 + 1);
      RawIndexArray->emplace_back(I * 6 + 4);
    }

    break;
  }

  case LAYOUT_TYPE::NORMAL_TEX: {
    std::vector<vertex_type::BasicVertex> *BasicArray =
        static_cast<std::vector<vertex_type::BasicVertex> *>(RawVertexArray);
    std::vector<vertex_type::BasicVertex> BasicCopy = *BasicArray;
    std::vector<UINT> IndexCopy = *RawIndexArray;

    BasicArray->resize(0);
    RawIndexArray->resize(0);

    UINT NumTris = static_cast<UINT>(IndexCopy.size()) / 3;
    for (UINT I = 0; I < NumTris; I++) {
      vertex_type::BasicVertex V0 = BasicCopy[IndexCopy[I * 3 + 0]];
      vertex_type::BasicVertex V1 = BasicCopy[IndexCopy[I * 3 + 1]];
      vertex_type::BasicVertex V2 = BasicCopy[IndexCopy[I * 3 + 2]];

      vertex_type::BasicVertex M0 = createBasicMidPoint(V0, V1);
      vertex_type::BasicVertex M1 = createBasicMidPoint(V1, V2);
      vertex_type::BasicVertex M2 = createBasicMidPoint(V0, V2);

      BasicArray->emplace_back(V0);
      BasicArray->emplace_back(V1);
      BasicArray->emplace_back(V2);
      BasicArray->emplace_back(M0);
      BasicArray->emplace_back(M1);
      BasicArray->emplace_back(M2);
      RawIndexArray->emplace_back(I * 6 + 0);
      RawIndexArray->emplace_back(I * 6 + 3);
      RawIndexArray->emplace_back(I * 6 + 5);
      RawIndexArray->emplace_back(I * 6 + 3);
      RawIndexArray->emplace_back(I * 6 + 4);
      RawIndexArray->emplace_back(I * 6 + 5);
      RawIndexArray->emplace_back(I * 6 + 5);
      RawIndexArray->emplace_back(I * 6 + 4);
      RawIndexArray->emplace_back(I * 6 + 2);
      RawIndexArray->emplace_back(I * 6 + 3);
      RawIndexArray->emplace_back(I * 6 + 1);
      RawIndexArray->emplace_back(I * 6 + 4);
    }

    break;
  }

  case LAYOUT_TYPE::NORMAL_TANGENT_TEX: {
    std::vector<vertex_type::TangentVertex> *TangentArray =
        static_cast<std::vector<vertex_type::TangentVertex> *>(RawVertexArray);
    std::vector<vertex_type::TangentVertex> TangentCopy = *TangentArray;
    std::vector<UINT> IndexCopy = *RawIndexArray;

    TangentArray->resize(0);
    RawIndexArray->resize(0);

    UINT NumTris = static_cast<UINT>(IndexCopy.size()) / 3;
    for (UINT I = 0; I < NumTris; I++) {
      vertex_type::TangentVertex V0 = TangentCopy[IndexCopy[I * 3 + 0]];
      vertex_type::TangentVertex V1 = TangentCopy[IndexCopy[I * 3 + 1]];
      vertex_type::TangentVertex V2 = TangentCopy[IndexCopy[I * 3 + 2]];

      vertex_type::TangentVertex M0 = createTangentMidPoint(V0, V1);
      vertex_type::TangentVertex M1 = createTangentMidPoint(V1, V2);
      vertex_type::TangentVertex M2 = createTangentMidPoint(V0, V2);

      TangentArray->emplace_back(V0);
      TangentArray->emplace_back(V1);
      TangentArray->emplace_back(V2);
      TangentArray->emplace_back(M0);
      TangentArray->emplace_back(M1);
      TangentArray->emplace_back(M2);
      RawIndexArray->emplace_back(I * 6 + 0);
      RawIndexArray->emplace_back(I * 6 + 3);
      RawIndexArray->emplace_back(I * 6 + 5);
      RawIndexArray->emplace_back(I * 6 + 3);
      RawIndexArray->emplace_back(I * 6 + 4);
      RawIndexArray->emplace_back(I * 6 + 5);
      RawIndexArray->emplace_back(I * 6 + 5);
      RawIndexArray->emplace_back(I * 6 + 4);
      RawIndexArray->emplace_back(I * 6 + 2);
      RawIndexArray->emplace_back(I * 6 + 3);
      RawIndexArray->emplace_back(I * 6 + 1);
      RawIndexArray->emplace_back(I * 6 + 4);
    }

    break;
  }

  default:
    assert(false && "null layout");
  }
}

vertex_type::BasicVertex
RSGeometryGenerator::createBasicMidPoint(const vertex_type::BasicVertex &V0,
                                         const vertex_type::BasicVertex &V1) {
  dx::XMVECTOR P0 = dx::XMLoadFloat3(&V0.Position);
  dx::XMVECTOR P1 = dx::XMLoadFloat3(&V1.Position);
  dx::XMVECTOR N0 = dx::XMLoadFloat3(&V0.Normal);
  dx::XMVECTOR N1 = dx::XMLoadFloat3(&V1.Normal);
  dx::XMVECTOR Tex0 = dx::XMLoadFloat2(&V0.TexCoord);
  dx::XMVECTOR Tex1 = dx::XMLoadFloat2(&V1.TexCoord);
  dx::XMVECTOR Pos = 0.5f * (P0 + P1);
  dx::XMVECTOR Normal = dx::XMVector3Normalize(0.5f * (N0 + N1));
  dx::XMVECTOR Tex = 0.5f * (Tex0 + Tex1);

  vertex_type::BasicVertex V = {};
  dx::XMStoreFloat3(&V.Position, Pos);
  dx::XMStoreFloat3(&V.Normal, Normal);
  dx::XMStoreFloat2(&V.TexCoord, Tex);

  return V;
}

vertex_type::TangentVertex
RSGeometryGenerator::createTangentMidPoint(
    const vertex_type::TangentVertex &V0,
    const vertex_type::TangentVertex &V1) {
  dx::XMVECTOR P0 = dx::XMLoadFloat3(&V0.Position);
  dx::XMVECTOR P1 = dx::XMLoadFloat3(&V1.Position);
  dx::XMVECTOR N0 = dx::XMLoadFloat3(&V0.Normal);
  dx::XMVECTOR N1 = dx::XMLoadFloat3(&V1.Normal);
  dx::XMVECTOR Tan0 = dx::XMLoadFloat3(&V0.Tangent);
  dx::XMVECTOR Tan1 = dx::XMLoadFloat3(&V1.Tangent);
  dx::XMVECTOR Tex0 = dx::XMLoadFloat2(&V0.TexCoord);
  dx::XMVECTOR Tex1 = dx::XMLoadFloat2(&V1.TexCoord);
  dx::XMVECTOR Pos = 0.5f * (P0 + P1);
  dx::XMVECTOR Normal = dx::XMVector3Normalize(0.5f * (N0 + N1));
  dx::XMVECTOR TangentPtr = dx::XMVector3Normalize(0.5f * (Tan0 + Tan1));
  dx::XMVECTOR Tex = 0.5f * (Tex0 + Tex1);

  vertex_type::TangentVertex V = {};
  dx::XMStoreFloat3(&V.Position, Pos);
  dx::XMStoreFloat3(&V.Normal, Normal);
  dx::XMStoreFloat3(&V.Tangent, TangentPtr);
  dx::XMStoreFloat2(&V.TexCoord, Tex);

  return V;
}

vertex_type::ColorVertex
RSGeometryGenerator::createColorMidPoint(const vertex_type::ColorVertex &V0,
                                         const vertex_type::ColorVertex &V1) {
  dx::XMVECTOR P0 = dx::XMLoadFloat3(&V0.Position);
  dx::XMVECTOR P1 = dx::XMLoadFloat3(&V1.Position);
  dx::XMVECTOR N0 = dx::XMLoadFloat3(&V0.Normal);
  dx::XMVECTOR N1 = dx::XMLoadFloat3(&V1.Normal);
  dx::XMVECTOR Col0 = dx::XMLoadFloat4(&V0.Color);
  dx::XMVECTOR Col1 = dx::XMLoadFloat4(&V1.Color);
  dx::XMVECTOR Pos = 0.5f * (P0 + P1);
  dx::XMVECTOR ColorPtr = 0.5f * (Col0 + Col1);
  dx::XMVECTOR Normal = dx::XMVector3Normalize(0.5f * (N0 + N1));

  vertex_type::ColorVertex V = {};
  dx::XMStoreFloat3(&V.Position, Pos);
  dx::XMStoreFloat3(&V.Normal, Normal);
  dx::XMStoreFloat4(&V.Color, ColorPtr);

  return V;
}

void
RSGeometryGenerator::buildCylinderTopCap(float BottomRadius,
                                         float TopRadius,
                                         float Height,
                                         UINT SliceCount,
                                         UINT StackCount,
                                         LAYOUT_TYPE LayoutType,
                                         void *RawVertexArray,
                                         std::vector<UINT> *RawIndexArray,
                                         const dx::XMFLOAT4 &VertexColor) {
  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR: {
    std::vector<vertex_type::ColorVertex> *ColorArray =
        static_cast<std::vector<vertex_type::ColorVertex> *>(RawVertexArray);

    UINT BaseIndex = static_cast<UINT>(ColorArray->size());
    float Y = 0.5f * Height;
    float DTheta = 2.f * dx::XM_PI / SliceCount;

    for (UINT I = 0; I <= SliceCount; I++) {
      float X = TopRadius * cosf(I * DTheta);
      float Z = TopRadius * sinf(I * DTheta);
      float U = X / Height + 0.5f;
      float V = Z / Height + 0.5f;
      (void)U;
      (void)V;

      vertex_type::ColorVertex Vert = {
          {X, Y, Z}, {0.0f, 1.0f, 0.0f}, VertexColor};
      ColorArray->emplace_back(Vert);
    }

    vertex_type::ColorVertex Vert = {
        {0.0f, Y, 0.0f}, {0.0f, 1.0f, 0.0f}, VertexColor};
    ColorArray->emplace_back(Vert);

    UINT CenterIndex = static_cast<UINT>(ColorArray->size()) - 1;

    for (UINT I = 0; I < SliceCount; I++) {
      RawIndexArray->emplace_back(CenterIndex);
      RawIndexArray->emplace_back(BaseIndex + I + 1);
      RawIndexArray->emplace_back(BaseIndex + I);
    }

    break;
  }

  case LAYOUT_TYPE::NORMAL_TEX: {
    std::vector<vertex_type::BasicVertex> *BasicArray =
        static_cast<std::vector<vertex_type::BasicVertex> *>(RawVertexArray);

    UINT BaseIndex = static_cast<UINT>(BasicArray->size());
    float Y = 0.5f * Height;
    float DTheta = 2.f * dx::XM_PI / SliceCount;

    for (UINT I = 0; I <= SliceCount; I++) {
      float X = TopRadius * cosf(I * DTheta);
      float Z = TopRadius * sinf(I * DTheta);
      float U = X / Height + 0.5f;
      float V = Z / Height + 0.5f;

      vertex_type::BasicVertex Vert = {{X, Y, Z}, {0.0f, 1.0f, 0.0f}, {U, V}};
      BasicArray->emplace_back(Vert);
    }

    vertex_type::BasicVertex Vert = {
        {0.0f, Y, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}};
    BasicArray->emplace_back(Vert);

    UINT CenterIndex = static_cast<UINT>(BasicArray->size()) - 1;

    for (UINT I = 0; I < SliceCount; I++) {
      RawIndexArray->emplace_back(CenterIndex);
      RawIndexArray->emplace_back(BaseIndex + I + 1);
      RawIndexArray->emplace_back(BaseIndex + I);
    }

    break;
  }

  case LAYOUT_TYPE::NORMAL_TANGENT_TEX: {
    std::vector<vertex_type::TangentVertex> *TangentArray =
        static_cast<std::vector<vertex_type::TangentVertex> *>(RawVertexArray);

    UINT BaseIndex = static_cast<UINT>(TangentArray->size());
    float Y = 0.5f * Height;
    float DTheta = 2.f * dx::XM_PI / SliceCount;

    for (UINT I = 0; I <= SliceCount; I++) {
      float X = TopRadius * cosf(I * DTheta);
      float Z = TopRadius * sinf(I * DTheta);
      float U = X / Height + 0.5f;
      float V = Z / Height + 0.5f;

      vertex_type::TangentVertex Vert = {
          {X, Y, Z}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {U, V}};
      TangentArray->emplace_back(Vert);
    }

    vertex_type::TangentVertex Vert = {
        {0.0f, Y, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.5f}};
    TangentArray->emplace_back(Vert);

    UINT CenterIndex = static_cast<UINT>(TangentArray->size()) - 1;

    for (UINT I = 0; I < SliceCount; I++) {
      RawIndexArray->emplace_back(CenterIndex);
      RawIndexArray->emplace_back(BaseIndex + I + 1);
      RawIndexArray->emplace_back(BaseIndex + I);
    }

    break;
  }

  default:
    assert(false && "null layout");
  }
}

void
RSGeometryGenerator::buildCylinderBottomCap(float BottomRadius,
                                            float TopRadius,
                                            float Height,
                                            UINT SliceCount,
                                            UINT StackCount,
                                            LAYOUT_TYPE LayoutType,
                                            void *RawVertexArray,
                                            std::vector<UINT> *RawIndexArray,
                                            const dx::XMFLOAT4 &VertexColor) {
  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_COLOR: {
    std::vector<vertex_type::ColorVertex> *ColorArray =
        static_cast<std::vector<vertex_type::ColorVertex> *>(RawVertexArray);

    UINT BaseIndex = static_cast<UINT>(ColorArray->size());
    float Y = -0.5f * Height;
    float DTheta = 2.f * dx::XM_PI / SliceCount;

    for (UINT I = 0; I <= SliceCount; I++) {
      float X = BottomRadius * cosf(I * DTheta);
      float Z = BottomRadius * sinf(I * DTheta);
      float U = X / Height + 0.5f;
      float V = Z / Height + 0.5f;
      (void)U;
      (void)V;

      vertex_type::ColorVertex Vert = {
          {X, Y, Z}, {0.0f, -1.0f, 0.0f}, VertexColor};
      ColorArray->emplace_back(Vert);
    }

    vertex_type::ColorVertex Vert = {
        {0.0f, Y, 0.0f}, {0.0f, -1.0f, 0.0f}, VertexColor};
    ColorArray->emplace_back(Vert);

    UINT CenterIndex = static_cast<UINT>(ColorArray->size()) - 1;

    for (UINT I = 0; I < SliceCount; I++) {
      RawIndexArray->emplace_back(CenterIndex);
      RawIndexArray->emplace_back(BaseIndex + I);
      RawIndexArray->emplace_back(BaseIndex + I + 1);
    }

    break;
  }

  case LAYOUT_TYPE::NORMAL_TEX: {
    std::vector<vertex_type::BasicVertex> *BasicArray =
        static_cast<std::vector<vertex_type::BasicVertex> *>(RawVertexArray);

    UINT BaseIndex = static_cast<UINT>(BasicArray->size());
    float Y = -0.5f * Height;
    float DTheta = 2.f * dx::XM_PI / SliceCount;

    for (UINT I = 0; I <= SliceCount; I++) {
      float X = BottomRadius * cosf(I * DTheta);
      float Z = BottomRadius * sinf(I * DTheta);
      float U = X / Height + 0.5f;
      float V = Z / Height + 0.5f;

      vertex_type::BasicVertex Vert = {{X, Y, Z}, {0.0f, -1.0f, 0.0f}, {U, V}};
      BasicArray->emplace_back(Vert);
    }

    vertex_type::BasicVertex Vert = {
        {0.0f, Y, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}};
    BasicArray->emplace_back(Vert);

    UINT CenterIndex = static_cast<UINT>(BasicArray->size()) - 1;

    for (UINT I = 0; I < SliceCount; I++) {
      RawIndexArray->emplace_back(CenterIndex);
      RawIndexArray->emplace_back(BaseIndex + I);
      RawIndexArray->emplace_back(BaseIndex + I + 1);
    }

    break;
  }

  case LAYOUT_TYPE::NORMAL_TANGENT_TEX: {
    std::vector<vertex_type::TangentVertex> *TangentArray =
        static_cast<std::vector<vertex_type::TangentVertex> *>(RawVertexArray);

    UINT BaseIndex = static_cast<UINT>(TangentArray->size());
    float Y = -0.5f * Height;
    float DTheta = 2.f * dx::XM_PI / SliceCount;

    for (UINT I = 0; I <= SliceCount; I++) {
      float X = BottomRadius * cosf(I * DTheta);
      float Z = BottomRadius * sinf(I * DTheta);
      float U = X / Height + 0.5f;
      float V = Z / Height + 0.5f;

      vertex_type::TangentVertex Vert = {
          {X, Y, Z}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {U, V}};
      TangentArray->emplace_back(Vert);
    }

    vertex_type::TangentVertex Vert = {
        {0.0f, Y, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.5f}};
    TangentArray->emplace_back(Vert);

    UINT CenterIndex = static_cast<UINT>(TangentArray->size()) - 1;

    for (UINT I = 0; I < SliceCount; I++) {
      RawIndexArray->emplace_back(CenterIndex);
      RawIndexArray->emplace_back(BaseIndex + I);
      RawIndexArray->emplace_back(BaseIndex + I + 1);
    }

    break;
  }

  default:
    assert(false && "null layout");
  }
}

RS_SUBMESH_DATA
RSGeometryGenerator::createPointWithTexture(LAYOUT_TYPE LayoutType,
                                            const std::string &TextureName) {
  RS_SUBMESH_DATA RSD = {};
  SUBMESH_INFO SI = {};
  MATERIAL_INFO MI = {};
  std::vector<UINT> Indeices = {};
  std::vector<vertex_type::TangentVertex> BasicPtr = {};
  std::vector<std::string> Textures = {};
  std::string Str = "";

  Indeices.push_back(0);

  switch (LayoutType) {
  case LAYOUT_TYPE::NORMAL_TANGENT_TEX:
    BasicPtr.resize(1);
    BasicPtr[0] = {
        {0.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {1.f, 0.f, 0.f}, {0.0f, 0.0f}};

    Textures.emplace_back(TextureName);

    SI.TopologyType = TOPOLOGY_TYPE::POINTLIST;
    SI.VerteicesPtr = &BasicPtr;
    SI.IndeicesPtr = &Indeices;
    SI.TexturesPtr = &Textures;
    SI.MaterialPtr = &MI;
    MeshHelper->processSubMesh(&RSD, &SI, LayoutType);

    break;

  default:
    assert(false && "unvalid layout");
  }

  return RSD;
}
