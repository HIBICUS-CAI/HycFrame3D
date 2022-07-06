//---------------------------------------------------------------
// File: RSParticlesContainer.h
// Proj: RenderSystem_DX11
// Info: 保存并管理所有的粒子发射器
// Date: 2021.11.11
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include "RSParticleEmitter.h"

#include <unordered_map>

class RSParticlesContainer {
private:
  class RSRoot_DX11 *RenderSystemRoot;

  bool ResetFlag;

  std::vector<RSParticleEmitter *> ParticleEmitterArray;
  std::unordered_map<std::string, RSParticleEmitter *> ParticleEmitterMap;

  CRITICAL_SECTION DataLock;

public:
  RSParticlesContainer();
  ~RSParticlesContainer();

  bool
  startUp(class RSRoot_DX11 *RootPtr);

  void
  cleanAndStop();

  bool
  getResetFlg();

  void
  resetRSParticleSystem();

  void
  finishResetRSParticleSystem();

  RSParticleEmitter *
  createRSParticleEmitter(const std::string &Name,
                          const PARTICLE_EMITTER_INFO *Info);

  void
  deleteRSParticleEmitter(const std::string &Name);

  RSParticleEmitter *
  getRSParticleEmitter(const std::string &Name);

  std::vector<RSParticleEmitter *> *
  getAllParticleEmitters();

  void
  startRSParticleEmitter(const std::string &Name);

  void
  pauseRSParticleEmitter(const std::string &Name);

  inline void
  lockContainer() {
    EnterCriticalSection(&DataLock);
  }

  inline void
  unlockContainer() {
    LeaveCriticalSection(&DataLock);
  }
};
