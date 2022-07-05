//---------------------------------------------------------------
// File: RSPipelinesManager.h
// Proj: RenderSystem_DX11
// Info: 保存并管理所有的pipeline
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include <unordered_map>

class RSPipelinesManager {
private:
  class RSRoot_DX11 *RenderSystemRoot;
  class RSPipeline *CurrentPipeline;
  class RSPipeline *NextPipeline;
  std::unordered_map<std::string, class RSPipeline *> PipelineMap;
  CRITICAL_SECTION DataLock;

public:
  RSPipelinesManager();
  ~RSPipelinesManager();

  bool
  startUp(class RSRoot_DX11 *RootPtr);

  void
  cleanAndStop();

  void
  addPipeline(const std::string &Name, class RSPipeline *Pipeline);

  class RSPipeline *
  getPipeline(const std::string &Name);

  void
  setPipeline(const std::string &Name);

  void
  setPipeline(class RSPipeline *Pipeline);

  void
  clearCurrentPipelineState();

  void
  execuateCurrentPipeline();

  void
  useNextPipeline();
};
