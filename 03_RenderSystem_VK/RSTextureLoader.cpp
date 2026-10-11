//---------------------------------------------------------------
// File: RSTextureLoader.cpp
// Proj: RenderSystem_VK
// Info: 从文件读取纹理并创建Vulkan图像与着色器资源视图
// Date: 2026.10.11
// Comt: 替代DirectXTK的DDSTextureLoader与WICTextureLoader
//---------------------------------------------------------------

#include "RSTextureLoader.h"

#include "RSDevices.h"
#include "RSVkObjects.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO_WARNING
#include <stb/stb_image.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

namespace {

#pragma pack(push, 1)
struct DDS_PIXELFORMAT {
  uint32_t Size;
  uint32_t Flags;
  uint32_t FourCC;
  uint32_t RGBBitCount;
  uint32_t RBitMask;
  uint32_t GBitMask;
  uint32_t BBitMask;
  uint32_t ABitMask;
};

struct DDS_HEADER {
  uint32_t Size;
  uint32_t Flags;
  uint32_t Height;
  uint32_t Width;
  uint32_t PitchOrLinearSize;
  uint32_t Depth;
  uint32_t MipMapCount;
  uint32_t Reserved1[11];
  DDS_PIXELFORMAT PixelFormat;
  uint32_t Caps;
  uint32_t Caps2;
  uint32_t Caps3;
  uint32_t Caps4;
  uint32_t Reserved2;
};

struct DDS_HEADER_DXT10 {
  uint32_t DxgiFormat;
  uint32_t ResourceDimension;
  uint32_t MiscFlag;
  uint32_t ArraySize;
  uint32_t MiscFlags2;
};
#pragma pack(pop)

constexpr uint32_t DDS_MAGIC = 0x20534444; // "DDS "
constexpr uint32_t DDS_FOURCC = 0x00000004;
constexpr uint32_t DDS_RGB = 0x00000040;
constexpr uint32_t DDS_LUMINANCE = 0x00020000;
constexpr uint32_t DDS_HEADER_FLAGS_VOLUME = 0x00800000;
constexpr uint32_t DDS_CUBEMAP = 0x00000200;
constexpr uint32_t DDS_RESOURCE_MISC_TEXTURECUBE = 0x4;
constexpr uint32_t DDS_DIMENSION_TEXTURE3D = 4;

constexpr uint32_t makeFourCC(char A, char B, char C, char D) {
  return static_cast<uint32_t>(static_cast<uint8_t>(A)) |
         (static_cast<uint32_t>(static_cast<uint8_t>(B)) << 8) |
         (static_cast<uint32_t>(static_cast<uint8_t>(C)) << 16) |
         (static_cast<uint32_t>(static_cast<uint8_t>(D)) << 24);
}

VkFormat dxgiToVkFormat(uint32_t Dxgi) {
  switch (Dxgi) {
  case 2:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  case 3:
    return VK_FORMAT_R32G32B32A32_UINT;
  case 6:
    return VK_FORMAT_R32G32B32_SFLOAT;
  case 10:
    return VK_FORMAT_R16G16B16A16_SFLOAT;
  case 11:
    return VK_FORMAT_R16G16B16A16_UNORM;
  case 16:
    return VK_FORMAT_R32G32_SFLOAT;
  case 24:
    return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
  case 26:
    return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
  case 28:
    return VK_FORMAT_R8G8B8A8_UNORM;
  case 29:
    return VK_FORMAT_R8G8B8A8_SRGB;
  case 34:
    return VK_FORMAT_R16G16_SFLOAT;
  case 35:
    return VK_FORMAT_R16G16_UNORM;
  case 41:
    return VK_FORMAT_R32_SFLOAT;
  case 42:
    return VK_FORMAT_R32_UINT;
  case 49:
    return VK_FORMAT_R8G8_UNORM;
  case 54:
    return VK_FORMAT_R16_SFLOAT;
  case 56:
    return VK_FORMAT_R16_UNORM;
  case 61:
    return VK_FORMAT_R8_UNORM;
  case 71:
    return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
  case 72:
    return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
  case 74:
    return VK_FORMAT_BC2_UNORM_BLOCK;
  case 75:
    return VK_FORMAT_BC2_SRGB_BLOCK;
  case 77:
    return VK_FORMAT_BC3_UNORM_BLOCK;
  case 78:
    return VK_FORMAT_BC3_SRGB_BLOCK;
  case 80:
    return VK_FORMAT_BC4_UNORM_BLOCK;
  case 81:
    return VK_FORMAT_BC4_SNORM_BLOCK;
  case 83:
    return VK_FORMAT_BC5_UNORM_BLOCK;
  case 84:
    return VK_FORMAT_BC5_SNORM_BLOCK;
  case 87:
  case 88:
    return VK_FORMAT_B8G8R8A8_UNORM;
  case 91:
  case 93:
    return VK_FORMAT_B8G8R8A8_SRGB;
  case 95:
    return VK_FORMAT_BC6H_UFLOAT_BLOCK;
  case 96:
    return VK_FORMAT_BC6H_SFLOAT_BLOCK;
  case 98:
    return VK_FORMAT_BC7_UNORM_BLOCK;
  case 99:
    return VK_FORMAT_BC7_SRGB_BLOCK;
  default:
    return VK_FORMAT_UNDEFINED;
  }
}

VkFormat legacyToVkFormat(const DDS_PIXELFORMAT &Format) {
  if (Format.Flags & DDS_FOURCC) {
    switch (Format.FourCC) {
    case makeFourCC('D', 'X', 'T', '1'):
      return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case makeFourCC('D', 'X', 'T', '2'):
    case makeFourCC('D', 'X', 'T', '3'):
      return VK_FORMAT_BC2_UNORM_BLOCK;
    case makeFourCC('D', 'X', 'T', '4'):
    case makeFourCC('D', 'X', 'T', '5'):
      return VK_FORMAT_BC3_UNORM_BLOCK;
    case makeFourCC('A', 'T', 'I', '1'):
    case makeFourCC('B', 'C', '4', 'U'):
      return VK_FORMAT_BC4_UNORM_BLOCK;
    case makeFourCC('B', 'C', '4', 'S'):
      return VK_FORMAT_BC4_SNORM_BLOCK;
    case makeFourCC('A', 'T', 'I', '2'):
    case makeFourCC('B', 'C', '5', 'U'):
      return VK_FORMAT_BC5_UNORM_BLOCK;
    case makeFourCC('B', 'C', '5', 'S'):
      return VK_FORMAT_BC5_SNORM_BLOCK;
    // D3DFORMAT values stored in the fourcc field
    case 36:
      return VK_FORMAT_R16G16B16A16_UNORM;
    case 111:
      return VK_FORMAT_R16_SFLOAT;
    case 112:
      return VK_FORMAT_R16G16_SFLOAT;
    case 113:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
    case 114:
      return VK_FORMAT_R32_SFLOAT;
    case 115:
      return VK_FORMAT_R32G32_SFLOAT;
    case 116:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    default:
      return VK_FORMAT_UNDEFINED;
    }
  }

  if (Format.Flags & DDS_RGB) {
    if (Format.RGBBitCount == 32) {
      if (Format.RBitMask == 0x000000ff && Format.GBitMask == 0x0000ff00 &&
          Format.BBitMask == 0x00ff0000) {
        return VK_FORMAT_R8G8B8A8_UNORM;
      }
      if (Format.RBitMask == 0x00ff0000 && Format.GBitMask == 0x0000ff00 &&
          Format.BBitMask == 0x000000ff) {
        return VK_FORMAT_B8G8R8A8_UNORM;
      }
      if (Format.RBitMask == 0x0000ffff && Format.GBitMask == 0xffff0000) {
        return VK_FORMAT_R16G16_UNORM;
      }
      if (Format.RBitMask == 0xffffffff) {
        return VK_FORMAT_R32_SFLOAT;
      }
    }
  }

  if ((Format.Flags & DDS_LUMINANCE) && Format.RGBBitCount == 8) {
    return VK_FORMAT_R8_UNORM;
  }

  return VK_FORMAT_UNDEFINED;
}

bool readFile(const std::string &FileName, std::vector<uint8_t> &OutData) {
  std::ifstream File(FileName, std::ios::binary | std::ios::ate);
  if (!File) {
    return false;
  }
  auto Size = File.tellg();
  File.seekg(0, std::ios::beg);
  OutData.resize(static_cast<size_t>(Size));
  return static_cast<bool>(
      File.read(reinterpret_cast<char *>(OutData.data()), Size));
}

void outputResult(RSTexture *Texture,
                  RSShaderResourceView *Srv,
                  RSTexture **OutTexture,
                  RSShaderResourceView **OutSrv) {
  if (OutSrv) {
    *OutSrv = Srv;
  } else if (Srv) {
    Srv->Release();
  }
  if (OutTexture) {
    *OutTexture = Texture;
  } else if (Texture) {
    Texture->Release();
  }
}

} // namespace

namespace rs_tool {

bool createDDSTextureFromFile(RSDevices *Devices,
                              const std::string &FileName,
                              RSTexture **OutTexture,
                              RSShaderResourceView **OutSrv) {
  std::vector<uint8_t> Data = {};
  if (!readFile(FileName, Data) ||
      Data.size() < sizeof(uint32_t) + sizeof(DDS_HEADER)) {
    return false;
  }

  uint32_t Magic = 0;
  std::memcpy(&Magic, Data.data(), sizeof(Magic));
  if (Magic != DDS_MAGIC) {
    return false;
  }
  DDS_HEADER Header = {};
  std::memcpy(&Header, Data.data() + sizeof(uint32_t), sizeof(Header));
  size_t Offset = sizeof(uint32_t) + sizeof(DDS_HEADER);

  RS_TEXTURE_DESC Desc = {};
  Desc.Width = std::max(1u, Header.Width);
  Desc.Height = std::max(1u, Header.Height);
  Desc.Depth = 1;
  Desc.MipLevels = std::max(1u, Header.MipMapCount);
  Desc.ArrayLayers = 1;
  Desc.Usage = VK_IMAGE_USAGE_SAMPLED_BIT;

  if ((Header.PixelFormat.Flags & DDS_FOURCC) &&
      Header.PixelFormat.FourCC == makeFourCC('D', 'X', '1', '0')) {
    if (Data.size() < Offset + sizeof(DDS_HEADER_DXT10)) {
      return false;
    }
    DDS_HEADER_DXT10 Header10 = {};
    std::memcpy(&Header10, Data.data() + Offset, sizeof(Header10));
    Offset += sizeof(DDS_HEADER_DXT10);
    Desc.Format = dxgiToVkFormat(Header10.DxgiFormat);
    Desc.ArrayLayers = std::max(1u, Header10.ArraySize);
    if (Header10.ResourceDimension == DDS_DIMENSION_TEXTURE3D) {
      Desc.Type = VK_IMAGE_TYPE_3D;
      Desc.Depth = std::max(1u, Header.Depth);
    }
    if (Header10.MiscFlag & DDS_RESOURCE_MISC_TEXTURECUBE) {
      Desc.ArrayLayers *= 6;
      Desc.CubeCompatible = true;
    }
  } else {
    Desc.Format = legacyToVkFormat(Header.PixelFormat);
    if (Header.Flags & DDS_HEADER_FLAGS_VOLUME) {
      Desc.Type = VK_IMAGE_TYPE_3D;
      Desc.Depth = std::max(1u, Header.Depth);
    }
    if (Header.Caps2 & DDS_CUBEMAP) {
      Desc.ArrayLayers = 6;
      Desc.CubeCompatible = true;
    }
  }

  if (Desc.Format == VK_FORMAT_UNDEFINED) {
    std::fprintf(stderr, "[[[WARNING]]] : unsupported dds format : %s\n",
                 FileName.c_str());
    return false;
  }

  RS_FORMAT_INFO Info = getFormatInfo(Desc.Format);
  std::vector<RS_SUBRESOURCE_DATA> Subresources = {};
  for (uint32_t Layer = 0; Layer < Desc.ArrayLayers; Layer++) {
    for (uint32_t Mip = 0; Mip < Desc.MipLevels; Mip++) {
      uint32_t W = std::max(1u, Desc.Width >> Mip);
      uint32_t H = std::max(1u, Desc.Height >> Mip);
      uint32_t D = std::max(1u, Desc.Depth >> Mip);
      uint32_t BlocksX = (W + Info.BlockWidth - 1) / Info.BlockWidth;
      uint32_t BlocksY = (H + Info.BlockHeight - 1) / Info.BlockHeight;
      RS_SUBRESOURCE_DATA Sub = {};
      Sub.RowPitch = BlocksX * Info.BlockSize;
      Sub.SlicePitch = Sub.RowPitch * BlocksY;
      size_t Size = static_cast<size_t>(Sub.SlicePitch) * D;
      if (Offset + Size > Data.size()) {
        std::fprintf(stderr, "[[[WARNING]]] : dds file is truncated : %s\n",
                     FileName.c_str());
        return false;
      }
      Sub.Data = Data.data() + Offset;
      Offset += Size;
      Subresources.push_back(Sub);
    }
  }

  RSTexture *Texture = Devices->createTexture(Desc, Subresources.data());
  if (!Texture) {
    return false;
  }
  RSShaderResourceView *Srv = Devices->createShaderResourceView(Texture);
  if (!Srv) {
    Texture->Release();
    return false;
  }

  outputResult(Texture, Srv, OutTexture, OutSrv);
  return true;
}

bool createImageTextureFromFile(RSDevices *Devices,
                                const std::string &FileName,
                                RSTexture **OutTexture,
                                RSShaderResourceView **OutSrv) {
  int Width = 0;
  int Height = 0;
  int Components = 0;
  stbi_uc *Pixels =
      stbi_load(FileName.c_str(), &Width, &Height, &Components, STBI_rgb_alpha);
  if (!Pixels) {
    return false;
  }

  RS_TEXTURE_DESC Desc = {};
  Desc.Format = VK_FORMAT_R8G8B8A8_UNORM;
  Desc.Width = static_cast<uint32_t>(Width);
  Desc.Height = static_cast<uint32_t>(Height);
  Desc.Usage = VK_IMAGE_USAGE_SAMPLED_BIT;

  RS_SUBRESOURCE_DATA Sub = {};
  Sub.Data = Pixels;
  Sub.RowPitch = static_cast<uint32_t>(Width) * 4;
  Sub.SlicePitch = Sub.RowPitch * static_cast<uint32_t>(Height);

  RSTexture *Texture = Devices->createTexture(Desc, &Sub);
  stbi_image_free(Pixels);
  if (!Texture) {
    return false;
  }
  RSShaderResourceView *Srv = Devices->createShaderResourceView(Texture);
  if (!Srv) {
    Texture->Release();
    return false;
  }

  outputResult(Texture, Srv, OutTexture, OutSrv);
  return true;
}

bool createTextureFromFile(RSDevices *Devices,
                           const std::string &FileName,
                           RSTexture **OutTexture,
                           RSShaderResourceView **OutSrv) {
  if (FileName.find(".dds") != std::string::npos ||
      FileName.find(".DDS") != std::string::npos) {
    return createDDSTextureFromFile(Devices, FileName, OutTexture, OutSrv);
  }
  return createImageTextureFromFile(Devices, FileName, OutTexture, OutSrv);
}

} // namespace rs_tool
