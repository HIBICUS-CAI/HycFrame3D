//---------------------------------------------------------------
// File: RSMeshHelper.h
// Proj: RenderSystem_DX11
// Info: 提供对SubMesh转换为DirectX可识别形式的方法
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

class RSMeshHelper {
private:
  class RSRoot_DX11 *RenderSystemRoot;
  class RSResourceManager *ResourceManager;
  class RSDevices *Devices;
  class RSGeometryGenerator *GeoGenerator;

public:
  RSMeshHelper();
  ~RSMeshHelper();

  bool
  startUp(class RSRoot_DX11 *RootPtr, class RSResourceManager *ResManagerPtr);

  void
  cleanAndStop();

  void
  processSubMesh(RS_SUBMESH_DATA *OutResult,
                 const SUBMESH_INFO *Info,
                 LAYOUT_TYPE LayoutType);

  void
  releaseSubMesh(RS_SUBMESH_DATA &MeshData);

  class RSGeometryGenerator *
  getGeoGenerator();

private:
  ID3D11InputLayout *
  refStaticInputLayout(LAYOUT_TYPE LayoutType);

  ID3D11Buffer *
  createIndexBuffer(const std::vector<UINT> *const IndicesArray);

  ID3D11Buffer *
  createVertexBuffer(const void *const ConstRawVerticesPtr,
                     LAYOUT_TYPE LayoutType);

  void
  createTexSrv(RS_SUBMESH_DATA *OutResult,
               const std::vector<std::string> *const Textures);

  void
  createSubMeshMaterial(RS_SUBMESH_DATA *OutResult,
                        const MATERIAL_INFO *const Info);
};

class RSGeometryGenerator {
private:
  RSMeshHelper *MeshHelper;
  class RSDevices *Devices;
  class RSResourceManager *ResourceManager;

public:
  RSGeometryGenerator(class RSRoot_DX11 *RootPtr);
  ~RSGeometryGenerator();

  RS_SUBMESH_DATA
  createBox(float Width,
            float Height,
            float Depth,
            UINT DivideNumber,
            LAYOUT_TYPE LayoutType,
            bool EnabledVertexColorFlag = true,
            const dx::XMFLOAT4 &VertexColor = {1.f, 1.f, 1.f, 1.f},
            const std::string &TextureName = "");

  RS_SUBMESH_DATA
  createSphere(float Radius,
               UINT SliceCount,
               UINT StackCount,
               LAYOUT_TYPE LayoutType,
               bool EnabledVertexColorFlag = true,
               const dx::XMFLOAT4 &VertexColor = {1.f, 1.f, 1.f, 1.f},
               const std::string &TextureName = "");

  RS_SUBMESH_DATA
  createGeometrySphere(float Radius,
                       UINT DivideNumber,
                       LAYOUT_TYPE LayoutType,
                       bool EnabledVertexColorFlag = true,
                       const dx::XMFLOAT4 &VertexColor = {1.f, 1.f, 1.f, 1.f},
                       const std::string &TextureName = "");

  RS_SUBMESH_DATA
  createCylinder(float BottomRadius,
                 float TopRadius,
                 float Height,
                 UINT SliceCount,
                 UINT StackCount,
                 LAYOUT_TYPE LayoutType,
                 bool EnabledVertexColorFlag = true,
                 const dx::XMFLOAT4 &VertexColor = {1.f, 1.f, 1.f, 1.f},
                 const std::string &TextureName = "");

  RS_SUBMESH_DATA
  createGrid(float Width,
             float Depth,
             UINT RowCount,
             UINT ColCount,
             LAYOUT_TYPE LayoutType,
             bool EnabledVertexColorFlag = true,
             const dx::XMFLOAT4 &VertexColor = {1.f, 1.f, 1.f, 1.f},
             const std::string &TextureName = "");

  RS_SUBMESH_DATA
  createSpriteRect(LAYOUT_TYPE LayoutType,
                   const std::string &TexturePath = "");

  RS_SUBMESH_DATA
  createPointWithTexture(LAYOUT_TYPE LayoutType,
                         const std::string &TextureName = "");

private:
  void
  processSubDivide(LAYOUT_TYPE LayoutType,
                   void *RawVertexArray,
                   std::vector<UINT> *RawIndexArray);

  vertex_type::BasicVertex
  createBasicMidPoint(const vertex_type::BasicVertex &V0,
                      const vertex_type::BasicVertex &V1);

  vertex_type::TangentVertex
  createTangentMidPoint(const vertex_type::TangentVertex &V0,
                        const vertex_type::TangentVertex &V1);

  vertex_type::ColorVertex
  createColorMidPoint(const vertex_type::ColorVertex &V0,
                      const vertex_type::ColorVertex &V1);

  void
  buildCylinderTopCap(float BottomRadius,
                      float TopRadius,
                      float Height,
                      UINT SliceCount,
                      UINT StackCount,
                      LAYOUT_TYPE LayoutType,
                      void *RawVertexArray,
                      std::vector<UINT> *RawIndexArray,
                      const dx::XMFLOAT4 &VertexColor);

  void
  buildCylinderBottomCap(float BottomRadius,
                         float TopRadius,
                         float Height,
                         UINT SliceCount,
                         UINT StackCount,
                         LAYOUT_TYPE LayoutType,
                         void *RawVertexArray,
                         std::vector<UINT> *RawIndexArray,
                         const dx::XMFLOAT4 &VertexColor);
};
