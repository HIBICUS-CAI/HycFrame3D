#pragma once

#include <Windows.h>
#include <d3d11_1.h>

namespace rs_tool {

HRESULT compileShaderFromFile(const WCHAR *FileName,
                              LPCSTR EntryPoint,
                              LPCSTR ShaderModel,
                              ID3DBlob **OutBlob,
                              const D3D_SHADER_MACRO *macro = nullptr);

HRESULT compileShaderFromFile(LPCSTR FileName,
                              LPCSTR EntryPoint,
                              LPCSTR ShaderModel,
                              ID3DBlob **OutBlob,
                              const D3D_SHADER_MACRO *Macro = nullptr);

} // namespace rs_tool
