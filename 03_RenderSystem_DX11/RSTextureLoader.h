//---------------------------------------------------------------
// File: RSTextureLoader.h
// Proj: RenderSystem_VK
// Info: 从文件读取纹理并创建Vulkan图像与着色器资源视图
// Date: 2026.10.11
// Comt: 替代DirectXTK的DDSTextureLoader与WICTextureLoader
//---------------------------------------------------------------

#pragma once

#include <string>

class RSDevices;
class RSTexture;
class RSShaderResourceView;

namespace rs_tool {

// dds files with legacy or dx10 headers, keeps mips, arrays and cube maps.
// OutTexture is optional, the view keeps the texture alive.
bool createDDSTextureFromFile(RSDevices *Devices,
                              const std::string &FileName,
                              RSTexture **OutTexture,
                              RSShaderResourceView **OutSrv);

// png/jpg/bmp/tga... decoded to rgba8 unorm without mips, like
// CreateWICTextureFromFile without a device context
bool createImageTextureFromFile(RSDevices *Devices,
                                const std::string &FileName,
                                RSTexture **OutTexture,
                                RSShaderResourceView **OutSrv);

// chooses one of the above by the file extension
bool createTextureFromFile(RSDevices *Devices,
                           const std::string &FileName,
                           RSTexture **OutTexture,
                           RSShaderResourceView **OutSrv);

} // namespace rs_tool
