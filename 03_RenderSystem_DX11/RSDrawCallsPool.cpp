//---------------------------------------------------------------
// File: RSDrawCallsPool.cpp
// Proj: RenderSystem_DX11
// Info: 管理所有保存着绘制图元的通道
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSDrawCallsPool.h"

#include "RSRoot_DX11.h"

RSDrawCallsPool::RSDrawCallsPool()
    : RenderSystemRoot(nullptr), DrawCallsPipeArray({{}}) {}

RSDrawCallsPool::~RSDrawCallsPool() {}

bool
RSDrawCallsPool::startUp(RSRoot_DX11 *RootPtr) {
  if (!RootPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;

  return true;
}

void
RSDrawCallsPool::cleanAndStop() {
  for (auto &Pipe : DrawCallsPipeArray) {
    Pipe.Data.clear();
  }
}

void
RSDrawCallsPool::addDrawCallToPipe(DRAWCALL_TYPE Type,
                                   const RS_DRAWCALL_DATA &DrawCall) {
  DrawCallsPipeArray[static_cast<size_t>(Type)].Data.emplace_back(DrawCall);
}

RSDrawCallsPipe *
RSDrawCallsPool::getDrawCallsPipe(DRAWCALL_TYPE Type) {
  return &DrawCallsPipeArray[static_cast<size_t>(Type)];
}

void
RSDrawCallsPool::clearAllDrawCalls() {
  for (auto &Pipe : DrawCallsPipeArray) {
    Pipe.Data.clear();
  }
}
