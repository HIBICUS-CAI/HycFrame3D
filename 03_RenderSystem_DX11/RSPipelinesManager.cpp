//---------------------------------------------------------------
// File: RSPipelinesManager.cpp
// Proj: RenderSystem_DX11
// Info: 保存并管理所有的pipeline
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSPipelinesManager.h"

#include "RSLightsContainer.h"
#include "RSParticlesContainer.h"
#include "RSPipeline.h"
#include "RSRoot_DX11.h"

#define LOCK EnterCriticalSection(&DataLock)
#define UNLOCK LeaveCriticalSection(&DataLock)

RSPipelinesManager::RSPipelinesManager()
    : RenderSystemRoot(nullptr), CurrentPipeline(nullptr),
      NextPipeline(nullptr), PipelineMap({}), DataLock({}) {}

RSPipelinesManager::~RSPipelinesManager() {}

bool
RSPipelinesManager::startUp(RSRoot_DX11 *RootPtr) {
  if (!RootPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;
  InitializeCriticalSection(&DataLock);

  return true;
}

void
RSPipelinesManager::cleanAndStop() {
  NextPipeline = nullptr;
  CurrentPipeline = nullptr;

  for (auto &Pipeline : PipelineMap) {
    Pipeline.second->releasePipeline();
    delete Pipeline.second;
  }

  PipelineMap.clear();
  DeleteCriticalSection(&DataLock);
}

void
RSPipelinesManager::addPipeline(const std::string &Name, RSPipeline *Pipeline) {
  LOCK;
  if (PipelineMap.find(Name) == PipelineMap.end()) {
    PipelineMap.insert({Name, Pipeline});
  }
  UNLOCK;
}

RSPipeline *
RSPipelinesManager::getPipeline(const std::string &Name) {
  LOCK;
  auto Found = PipelineMap.find(Name);
  if (Found != PipelineMap.end()) {
    auto Pipeline = Found->second;
    UNLOCK;
    return Pipeline;
  } else {
    UNLOCK;
    return nullptr;
  }
}

void
RSPipelinesManager::setPipeline(const std::string &Name) {
  LOCK;
  auto Found = PipelineMap.find(Name);
  if (Found != PipelineMap.end()) {
    NextPipeline = Found->second;
  }
  UNLOCK;
}

void
RSPipelinesManager::setPipeline(RSPipeline *Pipeline) {
  NextPipeline = Pipeline;
}

void
RSPipelinesManager::clearCurrentPipelineState() {
  CurrentPipeline = nullptr;
  NextPipeline = nullptr;
}

void
RSPipelinesManager::execuateCurrentPipeline() {
  RenderSystemRoot->getLightsContainer()->lockContainer();
  RenderSystemRoot->getParticlesContainer()->lockContainer();
  CurrentPipeline->execuatePipeline();
  RenderSystemRoot->getLightsContainer()->unlockContainer();
  RenderSystemRoot->getParticlesContainer()->unlockContainer();
}

void
RSPipelinesManager::useNextPipeline() {
  if (NextPipeline) {
    if (CurrentPipeline) {
      CurrentPipeline->suspendAllThread();
    }
    CurrentPipeline = NextPipeline;
    CurrentPipeline->resumeAllThread();
    NextPipeline = nullptr;
  }
}
