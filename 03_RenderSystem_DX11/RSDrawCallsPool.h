//---------------------------------------------------------------
// File: RSDrawCallsPool.h
// Proj: RenderSystem_DX11
// Info: 管理所有保存着绘制图元的通道
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include <array>

class RSDrawCallsPool {
private:
  class RSRoot_DX11 *RenderSystemRoot;

  std::array<RSDrawCallsPipe, static_cast<size_t>(DRAWCALL_TYPE::MAX)>
      DrawCallsPipeArray;

public:
  RSDrawCallsPool();
  ~RSDrawCallsPool();

  bool
  startUp(class RSRoot_DX11 *RootPtr);

  void
  cleanAndStop();

  void
  addDrawCallToPipe(DRAWCALL_TYPE Type, const RS_DRAWCALL_DATA &DrawCall);

  RSDrawCallsPipe *
  getDrawCallsPipe(DRAWCALL_TYPE Type);

  void
  clearAllDrawCalls();
};
