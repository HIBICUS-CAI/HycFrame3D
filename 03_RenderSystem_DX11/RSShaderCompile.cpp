#include "RSShaderCompile.h"

#include <d3dcompiler.h>
#include <string>

HRESULT rs_tool::compileShaderFromFile(const WCHAR *FileName,
                                       LPCSTR EntryPoint,
                                       LPCSTR ShaderModel,
                                       ID3DBlob **OutBlob,
                                       const D3D_SHADER_MACRO *Macro) {
  HRESULT Hr = S_OK;

  DWORD ShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  ShaderFlags |= D3DCOMPILE_DEBUG;
  ShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ID3DBlob *ErrorBlobPtr = nullptr;

  Hr = D3DCompileFromFile(FileName, Macro, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                          EntryPoint, ShaderModel, ShaderFlags, 0, OutBlob,
                          &ErrorBlobPtr);
  if (FAILED(Hr)) {
    if (ErrorBlobPtr) {
      OutputDebugStringA(
          reinterpret_cast<const char *>(ErrorBlobPtr->GetBufferPointer()));
      ErrorBlobPtr->Release();
    }
    return Hr;
  }
  if (ErrorBlobPtr) {
    ErrorBlobPtr->Release();
  }

  return S_OK;
}

HRESULT rs_tool::compileShaderFromFile(LPCSTR FileName,
                                       LPCSTR EntryPoint,
                                       LPCSTR ShaderModel,
                                       ID3DBlob **OutBlob,
                                       const D3D_SHADER_MACRO *Macro) {
  std::string Path = std::string(FileName);
  std::wstring WPath = std::wstring(Path.begin(), Path.end());

  HRESULT Hr = S_OK;

  DWORD ShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  ShaderFlags |= D3DCOMPILE_DEBUG;
  ShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ID3DBlob *ErrorBlobPtr = nullptr;

  Hr = D3DCompileFromFile(WPath.c_str(), Macro,
                          D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint,
                          ShaderModel, ShaderFlags, 0, OutBlob, &ErrorBlobPtr);
  if (FAILED(Hr)) {
    if (ErrorBlobPtr) {
      OutputDebugStringA(
          reinterpret_cast<const char *>(ErrorBlobPtr->GetBufferPointer()));
      ErrorBlobPtr->Release();
    }
    return Hr;
  }
  if (ErrorBlobPtr) {
    ErrorBlobPtr->Release();
  }

  return S_OK;
}
