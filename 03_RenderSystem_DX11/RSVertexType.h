//---------------------------------------------------------------
// File: RSVertexType.h
// Proj: RenderSystem_DX11
// Info: 声明RenderSystem_DX11相关所使用的顶点类型
// Date: 2021.9.19
// Mail: cai_genkan@outlook.com
// Comt: 目前考虑仅在RenderSystem_DX11中使用
//---------------------------------------------------------------

#pragma once

#include <DirectXMath.h>

namespace dx = DirectX;

namespace vertex_type {

struct BasicVertex {
  dx::XMFLOAT3 Position;
  dx::XMFLOAT3 Normal;
  dx::XMFLOAT2 TexCoord;
};

struct ColorVertex {
  dx::XMFLOAT3 Position;
  dx::XMFLOAT3 Normal;
  dx::XMFLOAT4 Color;
};

struct TangentVertex {
  dx::XMFLOAT3 Position;
  dx::XMFLOAT3 Normal;
  dx::XMFLOAT3 Tangent;
  dx::XMFLOAT2 TexCoord;
};

struct AnimationVertex {
  dx::XMFLOAT3 Position;
  dx::XMFLOAT3 Normal;
  dx::XMFLOAT3 Tangent;
  dx::XMFLOAT2 TexCoord;
  dx::XMFLOAT4 Weight;
  dx::XMUINT4 BoneID;
};

} // namespace vertex_type
