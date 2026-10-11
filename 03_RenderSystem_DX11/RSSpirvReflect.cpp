//---------------------------------------------------------------
// File: RSSpirvReflect.cpp
// Proj: RenderSystem_VK
// Info: 解析并修改编译后的SPIR-V，提供管线创建所需的反射信息
// Date: 2026.10.11
// Comt: 只处理本项目HLSL着色器实际用到的指令
//---------------------------------------------------------------

#include "RSSpirvReflect.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace {

constexpr uint32_t SPIRV_HEADER_SIZE = 5;

// opcodes
constexpr uint32_t OP_EXTENSION = 10;
constexpr uint32_t OP_ENTRY_POINT = 15;
constexpr uint32_t OP_TYPE_BOOL = 20;
constexpr uint32_t OP_TYPE_INT = 21;
constexpr uint32_t OP_TYPE_FLOAT = 22;
constexpr uint32_t OP_TYPE_VECTOR = 23;
constexpr uint32_t OP_TYPE_IMAGE = 25;
constexpr uint32_t OP_TYPE_SAMPLER = 26;
constexpr uint32_t OP_TYPE_SAMPLED_IMAGE = 27;
constexpr uint32_t OP_TYPE_ARRAY = 28;
constexpr uint32_t OP_TYPE_RUNTIME_ARRAY = 29;
constexpr uint32_t OP_TYPE_STRUCT = 30;
constexpr uint32_t OP_TYPE_POINTER = 32;
constexpr uint32_t OP_VARIABLE = 59;
constexpr uint32_t OP_IMAGE_TEXEL_POINTER = 60;
constexpr uint32_t OP_NAME = 5;
constexpr uint32_t OP_DECORATE = 71;
constexpr uint32_t OP_DECORATE_ID = 332;
constexpr uint32_t OP_DECORATE_STRING = 5632;
constexpr uint32_t OP_MEMBER_DECORATE_STRING = 5633;

// decorations
constexpr uint32_t DEC_BLOCK = 2;
constexpr uint32_t DEC_BUFFER_BLOCK = 3;
constexpr uint32_t DEC_BUILTIN = 11;
constexpr uint32_t DEC_LOCATION = 30;
constexpr uint32_t DEC_BINDING = 33;
constexpr uint32_t DEC_COUNTER_BUFFER = 5634;
constexpr uint32_t DEC_USER_SEMANTIC = 5635;

// storage classes
constexpr uint32_t SC_UNIFORM_CONSTANT = 0;
constexpr uint32_t SC_INPUT = 1;
constexpr uint32_t SC_UNIFORM = 2;
constexpr uint32_t SC_OUTPUT = 3;
constexpr uint32_t SC_STORAGE_BUFFER = 12;

struct INSTRUCTION {
  size_t Offset = 0;
  uint32_t Opcode = 0;
  uint32_t WordCount = 0;
};

std::vector<INSTRUCTION> splitInstructions(const std::vector<uint32_t> &Code) {
  std::vector<INSTRUCTION> Result = {};
  size_t Offset = SPIRV_HEADER_SIZE;
  while (Offset < Code.size()) {
    INSTRUCTION Inst = {};
    Inst.Offset = Offset;
    Inst.Opcode = Code[Offset] & 0xFFFF;
    Inst.WordCount = Code[Offset] >> 16;
    if (!Inst.WordCount) {
      break;
    }
    Result.push_back(Inst);
    Offset += Inst.WordCount;
  }
  return Result;
}

std::string readString(const std::vector<uint32_t> &Code, size_t WordOffset) {
  std::string Str = "";
  const char *Chars = reinterpret_cast<const char *>(&Code[WordOffset]);
  size_t MaxChars = (Code.size() - WordOffset) * sizeof(uint32_t);
  for (size_t I = 0; I < MaxChars && Chars[I]; I++) {
    Str.push_back(Chars[I]);
  }
  return Str;
}

std::string toUpper(std::string Str) {
  std::transform(Str.begin(), Str.end(), Str.begin(), [](unsigned char C) {
    return static_cast<char>(std::toupper(C));
  });
  return Str;
}

struct TYPE_INFO {
  uint32_t Opcode = 0;
  std::vector<uint32_t> Operands = {};
};

struct MODULE_INFO {
  std::unordered_map<uint32_t, TYPE_INFO> Types = {};
  std::unordered_map<uint32_t, std::string> Names = {};
  std::unordered_map<uint32_t, uint32_t> Bindings = {};
  std::unordered_map<uint32_t, uint32_t> Locations = {};
  std::unordered_map<uint32_t, std::string> Semantics = {};
  std::unordered_set<uint32_t> BuiltIns = {};
  std::unordered_set<uint32_t> Blocks = {};
  std::unordered_set<uint32_t> BufferBlocks = {};
  // counter variable id -> owner variable id
  std::unordered_map<uint32_t, uint32_t> CounterOwners = {};
  struct VARIABLE {
    uint32_t Id = 0;
    uint32_t PointerType = 0;
    uint32_t StorageClass = 0;
  };
  std::vector<VARIABLE> Variables = {};
};

MODULE_INFO parseModule(const std::vector<uint32_t> &Code,
                        const std::vector<INSTRUCTION> &Insts) {
  MODULE_INFO Info = {};
  for (const auto &Inst : Insts) {
    const uint32_t *W = &Code[Inst.Offset];
    switch (Inst.Opcode) {
    case OP_NAME:
      Info.Names[W[1]] = readString(Code, Inst.Offset + 2);
      break;
    case OP_DECORATE:
      switch (W[2]) {
      case DEC_BINDING:
        Info.Bindings[W[1]] = W[3];
        break;
      case DEC_LOCATION:
        Info.Locations[W[1]] = W[3];
        break;
      case DEC_BUILTIN:
        Info.BuiltIns.insert(W[1]);
        break;
      case DEC_BLOCK:
        Info.Blocks.insert(W[1]);
        break;
      case DEC_BUFFER_BLOCK:
        Info.BufferBlocks.insert(W[1]);
        break;
      default:
        break;
      }
      break;
    case OP_DECORATE_ID:
      if (W[2] == DEC_COUNTER_BUFFER) {
        Info.CounterOwners[W[3]] = W[1];
      }
      break;
    case OP_DECORATE_STRING:
      if (W[2] == DEC_USER_SEMANTIC) {
        Info.Semantics[W[1]] = toUpper(readString(Code, Inst.Offset + 3));
      }
      break;
    case OP_TYPE_BOOL:
    case OP_TYPE_INT:
    case OP_TYPE_FLOAT:
    case OP_TYPE_VECTOR:
    case OP_TYPE_IMAGE:
    case OP_TYPE_SAMPLER:
    case OP_TYPE_SAMPLED_IMAGE:
    case OP_TYPE_ARRAY:
    case OP_TYPE_RUNTIME_ARRAY:
    case OP_TYPE_STRUCT: {
      TYPE_INFO Type = {};
      Type.Opcode = Inst.Opcode;
      Type.Operands.assign(W + 2, W + Inst.WordCount);
      Info.Types[W[1]] = Type;
      break;
    }
    case OP_TYPE_POINTER: {
      TYPE_INFO Type = {};
      Type.Opcode = Inst.Opcode;
      Type.Operands.assign(W + 2, W + Inst.WordCount);
      Info.Types[W[1]] = Type;
      break;
    }
    case OP_VARIABLE: {
      MODULE_INFO::VARIABLE Var = {};
      Var.PointerType = W[1];
      Var.Id = W[2];
      Var.StorageClass = W[3];
      Info.Variables.push_back(Var);
      break;
    }
    default:
      break;
    }
  }
  return Info;
}

rs_tool::SPIRV_SCALAR_KIND getScalarKind(const MODULE_INFO &Info,
                                         uint32_t TypeId) {
  auto Found = Info.Types.find(TypeId);
  if (Found == Info.Types.end()) {
    return rs_tool::SPIRV_SCALAR_KIND::FLOAT;
  }
  const auto &Type = Found->second;
  switch (Type.Opcode) {
  case OP_TYPE_VECTOR:
    return getScalarKind(Info, Type.Operands[0]);
  case OP_TYPE_INT:
    return Type.Operands[1] ? rs_tool::SPIRV_SCALAR_KIND::SINT
                            : rs_tool::SPIRV_SCALAR_KIND::UINT;
  default:
    return rs_tool::SPIRV_SCALAR_KIND::FLOAT;
  }
}

bool fillImageBinding(const MODULE_INFO &Info,
                      uint32_t ImageTypeId,
                      rs_tool::SPIRV_BINDING_INFO &Binding) {
  auto Found = Info.Types.find(ImageTypeId);
  if (Found == Info.Types.end() || Found->second.Opcode != OP_TYPE_IMAGE) {
    return false;
  }
  // sampled type, dim, depth, arrayed, ms, sampled, format
  const auto &Ops = Found->second.Operands;
  Binding.Scalar = getScalarKind(Info, Ops[0]);
  Binding.Arrayed = Ops[3] != 0;
  bool Storage = Ops[5] == 2;
  switch (Ops[1]) {
  case 0:
    Binding.Dim = rs_tool::SPIRV_IMAGE_DIM::TEX1D;
    break;
  case 1:
    Binding.Dim = rs_tool::SPIRV_IMAGE_DIM::TEX2D;
    break;
  case 2:
    Binding.Dim = rs_tool::SPIRV_IMAGE_DIM::TEX3D;
    break;
  case 3:
    Binding.Dim = rs_tool::SPIRV_IMAGE_DIM::CUBE;
    break;
  case 5:
    Binding.Dim = rs_tool::SPIRV_IMAGE_DIM::BUFFER;
    break;
  default:
    return false;
  }
  if (Binding.Dim == rs_tool::SPIRV_IMAGE_DIM::BUFFER) {
    Binding.Type = Storage ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                           : VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
  } else {
    Binding.Type = Storage ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                           : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  }
  return true;
}

} // namespace

namespace rs_tool {

void splitSemantic(const std::string &Semantic,
                   std::string &OutName,
                   uint32_t &OutIndex) {
  size_t Pos = Semantic.size();
  while (Pos > 0 && std::isdigit(static_cast<unsigned char>(Semantic[Pos - 1]))) {
    --Pos;
  }
  OutName = Semantic.substr(0, Pos);
  OutIndex = (Pos < Semantic.size())
                 ? static_cast<uint32_t>(std::stoul(Semantic.substr(Pos)))
                 : 0;
}

bool reflectSpirv(const std::vector<uint32_t> &Code,
                  SPIRV_REFLECTION &OutReflection) {
  if (Code.size() < SPIRV_HEADER_SIZE || Code[0] != 0x07230203) {
    return false;
  }

  auto Insts = splitInstructions(Code);
  MODULE_INFO Info = parseModule(Code, Insts);

  OutReflection = {};
  for (const auto &Inst : Insts) {
    if (Inst.Opcode != OP_ENTRY_POINT) {
      continue;
    }
    switch (Code[Inst.Offset + 1]) {
    case 0:
      OutReflection.Stage = VK_SHADER_STAGE_VERTEX_BIT;
      break;
    case 3:
      OutReflection.Stage = VK_SHADER_STAGE_GEOMETRY_BIT;
      break;
    case 4:
      OutReflection.Stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      break;
    case 5:
      OutReflection.Stage = VK_SHADER_STAGE_COMPUTE_BIT;
      break;
    default:
      return false;
    }
    OutReflection.EntryPoint = readString(Code, Inst.Offset + 3);
    break;
  }

  std::unordered_map<uint32_t, uint32_t> VarBindings = {};
  for (const auto &Var : Info.Variables) {
    auto PtrFound = Info.Types.find(Var.PointerType);
    if (PtrFound == Info.Types.end()) {
      continue;
    }
    uint32_t TypeId = PtrFound->second.Operands[1];

    if (Var.StorageClass == SC_INPUT || Var.StorageClass == SC_OUTPUT) {
      if (Info.BuiltIns.count(Var.Id) || !Info.Locations.count(Var.Id)) {
        continue;
      }
      SPIRV_IO_INFO Io = {};
      Io.Location = Info.Locations.at(Var.Id);
      auto SemFound = Info.Semantics.find(Var.Id);
      Io.Semantic = (SemFound != Info.Semantics.end()) ? SemFound->second : "";
      if (Var.StorageClass == SC_INPUT) {
        OutReflection.Inputs.push_back(Io);
      } else {
        OutReflection.Outputs.push_back(Io);
      }
      continue;
    }

    if (Var.StorageClass != SC_UNIFORM_CONSTANT &&
        Var.StorageClass != SC_UNIFORM &&
        Var.StorageClass != SC_STORAGE_BUFFER) {
      continue;
    }

    SPIRV_BINDING_INFO Binding = {};
    auto BindFound = Info.Bindings.find(Var.Id);
    if (BindFound == Info.Bindings.end()) {
      continue;
    }
    Binding.Binding = BindFound->second;
    auto NameFound = Info.Names.find(Var.Id);
    Binding.Name = (NameFound != Info.Names.end()) ? NameFound->second : "";

    // arrays of descriptors are not used by this project
    auto TypeFound = Info.Types.find(TypeId);
    if (TypeFound == Info.Types.end()) {
      continue;
    }
    const auto &Type = TypeFound->second;
    if (Type.Opcode == OP_TYPE_STRUCT) {
      if (Var.StorageClass == SC_STORAGE_BUFFER ||
          Info.BufferBlocks.count(TypeId)) {
        Binding.Type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      } else {
        Binding.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      }
    } else if (Type.Opcode == OP_TYPE_IMAGE) {
      if (!fillImageBinding(Info, TypeId, Binding)) {
        continue;
      }
    } else if (Type.Opcode == OP_TYPE_SAMPLER) {
      Binding.Type = VK_DESCRIPTOR_TYPE_SAMPLER;
    } else if (Type.Opcode == OP_TYPE_SAMPLED_IMAGE) {
      if (!fillImageBinding(Info, Type.Operands[0], Binding)) {
        continue;
      }
      Binding.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    } else {
      continue;
    }

    VarBindings[Var.Id] = Binding.Binding;
    OutReflection.Bindings.push_back(Binding);
  }

  // mark counter buffers
  for (const auto &Counter : Info.CounterOwners) {
    auto CounterBinding = VarBindings.find(Counter.first);
    auto OwnerBinding = VarBindings.find(Counter.second);
    if (CounterBinding == VarBindings.end() ||
        OwnerBinding == VarBindings.end()) {
      continue;
    }
    for (auto &Binding : OutReflection.Bindings) {
      if (Binding.Name == Info.Names[Counter.first] &&
          Binding.Binding == CounterBinding->second) {
        Binding.IsCounter = true;
        Binding.CounterOwnerBinding = OwnerBinding->second;
      }
    }
  }

  return OutReflection.Stage != VK_SHADER_STAGE_ALL;
}

void patchCounterBindings(std::vector<uint32_t> &Code, uint32_t Offset) {
  auto Insts = splitInstructions(Code);
  MODULE_INFO Info = parseModule(Code, Insts);

  for (const auto &Inst : Insts) {
    if (Inst.Opcode != OP_DECORATE || Code[Inst.Offset + 2] != DEC_BINDING) {
      continue;
    }
    uint32_t Target = Code[Inst.Offset + 1];
    auto Owner = Info.CounterOwners.find(Target);
    if (Owner == Info.CounterOwners.end()) {
      continue;
    }
    auto OwnerBinding = Info.Bindings.find(Owner->second);
    if (OwnerBinding != Info.Bindings.end()) {
      Code[Inst.Offset + 3] = OwnerBinding->second + Offset;
    }
  }
}

void fixAtomicImageFormats(std::vector<uint32_t> &Code) {
  constexpr uint32_t FORMAT_R32F = 3;
  constexpr uint32_t FORMAT_R32I = 24;
  constexpr uint32_t FORMAT_R32UI = 33;

  auto Insts = splitInstructions(Code);
  MODULE_INFO Info = parseModule(Code, Insts);

  std::unordered_map<uint32_t, uint32_t> VarPointerTypes = {};
  for (const auto &Var : Info.Variables) {
    VarPointerTypes[Var.Id] = Var.PointerType;
  }

  std::unordered_set<uint32_t> AtomicImageTypes = {};
  for (const auto &Inst : Insts) {
    if (Inst.Opcode != OP_IMAGE_TEXEL_POINTER) {
      continue;
    }
    auto PtrType = VarPointerTypes.find(Code[Inst.Offset + 3]);
    if (PtrType == VarPointerTypes.end()) {
      continue;
    }
    auto Pointer = Info.Types.find(PtrType->second);
    if (Pointer != Info.Types.end()) {
      AtomicImageTypes.insert(Pointer->second.Operands[1]);
    }
  }

  for (const auto &Inst : Insts) {
    if (Inst.Opcode != OP_TYPE_IMAGE ||
        !AtomicImageTypes.count(Code[Inst.Offset + 1])) {
      continue;
    }
    // result, sampled type, dim, depth, arrayed, ms, sampled, format
    uint32_t &Format = Code[Inst.Offset + 8];
    if (Format != 0) {
      continue;
    }
    switch (getScalarKind(Info, Code[Inst.Offset + 2])) {
    case rs_tool::SPIRV_SCALAR_KIND::SINT:
      Format = FORMAT_R32I;
      break;
    case rs_tool::SPIRV_SCALAR_KIND::UINT:
      Format = FORMAT_R32UI;
      break;
    default:
      Format = FORMAT_R32F;
      break;
    }
  }
}

void stripHlslFunctionality1(std::vector<uint32_t> &Code) {
  auto Insts = splitInstructions(Code);
  std::vector<uint32_t> Result(Code.begin(), Code.begin() + SPIRV_HEADER_SIZE);
  Result.reserve(Code.size());

  for (const auto &Inst : Insts) {
    bool Skip = false;
    switch (Inst.Opcode) {
    case OP_EXTENSION: {
      std::string Name = readString(Code, Inst.Offset + 1);
      Skip = (Name == "SPV_GOOGLE_hlsl_functionality1" ||
              Name == "SPV_GOOGLE_user_type");
      break;
    }
    case OP_DECORATE_STRING:
    case OP_MEMBER_DECORATE_STRING:
      Skip = true;
      break;
    case OP_DECORATE_ID:
      Skip = Code[Inst.Offset + 2] == DEC_COUNTER_BUFFER;
      break;
    default:
      break;
    }
    if (!Skip) {
      Result.insert(Result.end(), Code.begin() + Inst.Offset,
                    Code.begin() + Inst.Offset + Inst.WordCount);
    }
  }

  Code.swap(Result);
}

void remapLocations(std::vector<uint32_t> &Code,
                    bool ForInputs,
                    const std::unordered_map<std::string, uint32_t> &Locations) {
  auto Insts = splitInstructions(Code);
  MODULE_INFO Info = parseModule(Code, Insts);

  std::unordered_set<uint32_t> TargetVars = {};
  uint32_t TargetClass = ForInputs ? SC_INPUT : SC_OUTPUT;
  for (const auto &Var : Info.Variables) {
    if (Var.StorageClass == TargetClass) {
      TargetVars.insert(Var.Id);
    }
  }

  for (const auto &Inst : Insts) {
    if (Inst.Opcode != OP_DECORATE || Code[Inst.Offset + 2] != DEC_LOCATION) {
      continue;
    }
    uint32_t Target = Code[Inst.Offset + 1];
    if (!TargetVars.count(Target)) {
      continue;
    }
    auto Semantic = Info.Semantics.find(Target);
    if (Semantic == Info.Semantics.end()) {
      continue;
    }
    auto Location = Locations.find(Semantic->second);
    if (Location != Locations.end()) {
      Code[Inst.Offset + 3] = Location->second;
    }
  }
}

} // namespace rs_tool
