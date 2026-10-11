//---------------------------------------------------------------
// File: RSShaderCompile.cpp
// Proj: RenderSystem_VK
// Info: 在运行时将HLSL编译为SPIR-V
// Date: 2026.10.11
// Comt: 使用glslang的HLSL前端，保留原有的运行时宏注入方式
//---------------------------------------------------------------

#include "RSShaderCompile.h"

#include "RSSpirvReflect.h"

// hlsl front end apis of glslang are only declared with ENABLE_HLSL,
// the distributed glslang libraries are always built with it.
#ifndef ENABLE_HLSL
#define ENABLE_HLSL
#endif // !ENABLE_HLSL

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <cstdio>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>

namespace {

std::once_flag G_GlslangInitFlag = {};
bool G_GlslangInited = false;

bool readTextFile(const std::string &Path, std::string &OutText) {
  std::ifstream File(Path, std::ios::binary);
  if (!File) {
    return false;
  }
  std::stringstream Stream;
  Stream << File.rdbuf();
  OutText = Stream.str();
  // skip utf-8 bom
  if (OutText.size() >= 3 && static_cast<unsigned char>(OutText[0]) == 0xEF &&
      static_cast<unsigned char>(OutText[1]) == 0xBB &&
      static_cast<unsigned char>(OutText[2]) == 0xBF) {
    OutText.erase(0, 3);
  }
  return true;
}

std::string getDirectory(const std::string &Path) {
  auto Pos = Path.find_last_of("/\\");
  return (Pos == std::string::npos) ? std::string(".") : Path.substr(0, Pos);
}

// resolves #include relative to the including file, like
// D3D_COMPILE_STANDARD_FILE_INCLUDE
class ShaderIncluder : public glslang::TShader::Includer {
private:
  std::string RootDirectory;

public:
  ShaderIncluder(const std::string &RootDir) : RootDirectory(RootDir) {}

  IncludeResult *includeLocal(const char *HeaderName,
                              const char *IncluderName,
                              size_t InclusionDepth) override {
    std::string BaseDir = RootDirectory;
    if (InclusionDepth > 1 && IncluderName && IncluderName[0]) {
      BaseDir = getDirectory(IncluderName);
    }
    return openFile(BaseDir + "/" + HeaderName);
  }

  IncludeResult *includeSystem(const char *HeaderName,
                               const char *IncluderName,
                               size_t InclusionDepth) override {
    return includeLocal(HeaderName, IncluderName, InclusionDepth);
  }

  void releaseInclude(IncludeResult *Result) override {
    if (Result) {
      delete static_cast<std::string *>(Result->userData);
      delete Result;
    }
  }

private:
  IncludeResult *openFile(const std::string &Path) {
    auto Content = new std::string();
    if (!readTextFile(Path, *Content)) {
      delete Content;
      return nullptr;
    }
    return new IncludeResult(Path, Content->data(), Content->size(), Content);
  }
};

bool parseShaderModel(const std::string &Model,
                      EShLanguage &OutStage,
                      uint32_t &OutBindingBase) {
  if (Model.size() < 2) {
    return false;
  }
  std::string Prefix = Model.substr(0, 2);
  if (Prefix == "vs") {
    OutStage = EShLangVertex;
    OutBindingBase = RS_BINDING_VERTEX_BASE;
  } else if (Prefix == "gs") {
    OutStage = EShLangGeometry;
    OutBindingBase = RS_BINDING_GEOMETRY_BASE;
  } else if (Prefix == "ps") {
    OutStage = EShLangFragment;
    OutBindingBase = RS_BINDING_PIXEL_BASE;
  } else if (Prefix == "cs") {
    OutStage = EShLangCompute;
    OutBindingBase = RS_BINDING_COMPUTE_BASE;
  } else {
    return false;
  }
  return true;
}

void printError(const std::string &FileName, const char *Log) {
  std::fprintf(stderr, "[[[SHADER ERROR]]] : %s\n%s\n", FileName.c_str(),
               Log ? Log : "");
}

} // namespace

namespace rs_tool {

bool compileShaderFromFile(const std::string &FileName,
                           const std::string &EntryPoint,
                           const std::string &ShaderModel,
                           std::vector<uint32_t> &OutSpirv,
                           const SHADER_MACRO *Macros) {
  std::call_once(G_GlslangInitFlag,
                 []() { G_GlslangInited = glslang::InitializeProcess(); });
  if (!G_GlslangInited) {
    return false;
  }

  EShLanguage Stage = EShLangVertex;
  uint32_t Base = 0;
  if (!parseShaderModel(ShaderModel, Stage, Base)) {
    printError(FileName, "unsupported shader model");
    return false;
  }

  std::string Source = "";
  if (!readTextFile(FileName, Source)) {
    printError(FileName, "cannot open shader file");
    return false;
  }

  std::string Preamble = "";
  for (auto Macro = Macros; Macro && Macro->Name; Macro++) {
    Preamble += std::string("#define ") + Macro->Name + " " +
                (Macro->Definition ? Macro->Definition : "") + "\n";
  }

  glslang::TShader Shader(Stage);
  const char *SourcePtr = Source.c_str();
  const int SourceLength = static_cast<int>(Source.size());
  const char *SourceName = FileName.c_str();
  Shader.setStringsWithLengthsAndNames(&SourcePtr, &SourceLength, &SourceName,
                                       1);
  Shader.setPreamble(Preamble.c_str());
  Shader.setEntryPoint(EntryPoint.c_str());
  Shader.setSourceEntryPoint(EntryPoint.c_str());
  Shader.setEnvInput(glslang::EShSourceHlsl, Stage, glslang::EShClientVulkan,
                     100);
  Shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
  Shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
  // semantic strings and counter buffer links are needed by reflection
  Shader.setEnvTargetHlslFunctionality1();
  Shader.setHlslIoMapping(true);
  Shader.setNoStorageFormat(true);
  Shader.setShiftBinding(glslang::EResUbo, Base + RS_BINDING_CBV_OFFSET);
  Shader.setShiftBinding(glslang::EResTexture, Base + RS_BINDING_SRV_OFFSET);
  Shader.setShiftBinding(glslang::EResSsbo, Base + RS_BINDING_SRV_OFFSET);
  Shader.setShiftBinding(glslang::EResImage, Base + RS_BINDING_UAV_OFFSET);
  Shader.setShiftBinding(glslang::EResUav, Base + RS_BINDING_UAV_OFFSET);
  Shader.setShiftBinding(glslang::EResSampler,
                         Base + RS_BINDING_SAMPLER_OFFSET);

  EShMessages Messages = static_cast<EShMessages>(
      EShMsgSpvRules | EShMsgVulkanRules | EShMsgReadHlsl |
      EShMsgHlslOffsets | EShMsgHlslLegalization);

  ShaderIncluder Includer(getDirectory(FileName));
  if (!Shader.parse(GetDefaultResources(), 100, false, Messages, Includer)) {
    printError(FileName, Shader.getInfoLog());
    return false;
  }

  glslang::TProgram Program;
  Program.addShader(&Shader);
  if (!Program.link(Messages) || !Program.mapIO()) {
    printError(FileName, Program.getInfoLog());
    return false;
  }

  glslang::SpvOptions Options = {};
#ifdef _DEBUG
  Options.generateDebugInfo = true;
#endif // _DEBUG
  // the optimizer also legalizes hlsl (e.g. resources passed to functions)
  Options.disableOptimizer = false;
  Options.validate = false;
  spv::SpvBuildLogger Logger;
  OutSpirv.clear();
  glslang::GlslangToSpv(*Program.getIntermediate(Stage), OutSpirv, &Logger,
                        &Options);
  if (OutSpirv.empty()) {
    printError(FileName, Logger.getAllMessages().c_str());
    return false;
  }

  patchCounterBindings(OutSpirv, RS_BINDING_COUNTER_OFFSET);
  fixAtomicImageFormats(OutSpirv);

  return true;
}

void finalizeShaderCompiler() {
  if (G_GlslangInited) {
    glslang::FinalizeProcess();
    G_GlslangInited = false;
  }
}

} // namespace rs_tool
