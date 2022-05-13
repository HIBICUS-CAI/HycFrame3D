#pragma once

#include <Windows.h>
#include <d3d11_1.h>

namespace Tool
{
    HRESULT CompileShaderFromFile(const WCHAR* _fileName,
        LPCSTR _entryPoint, LPCSTR _shaderModel,
        ID3DBlob** _ppBlobOut,
        const D3D_SHADER_MACRO* _macro = nullptr);

    HRESULT CompileShaderFromFile(LPCSTR _fileName,
        LPCSTR _entryPoint, LPCSTR _shaderModel,
        ID3DBlob** _ppBlobOut,
        const D3D_SHADER_MACRO* _macro = nullptr);
}
