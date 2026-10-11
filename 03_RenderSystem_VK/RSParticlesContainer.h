//---------------------------------------------------------------
// File: RSParticlesContainer.h
// Proj: RenderSystem_VK
// Info: 保存并管理所有的粒子发射器
// Date: 2021.11.11
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include <mutex>

#include "RSParticleEmitter.h"

#include <unordered_map>

class RSParticlesContainer {
private:
  class RSRoot_VK *RenderSystemRoot;

  bool ResetFlag;

  std::vector<RSParticleEmitter *> ParticleEmitterArray;
  std::unordered_map<std::string, RSParticleEmitter *> ParticleEmitterMap;

  std::recursive_mutex DataLock;

public:
  RSParticlesContainer();
  ~RSParticlesContainer();

  bool startUp(class RSRoot_VK *RootPtr);
  void cleanAndStop();

  bool getResetFlg();
  void resetRSParticleSystem();
  void finishResetRSParticleSystem();

  RSParticleEmitter *createRSParticleEmitter(const std::string &Name,
                                             const PARTICLE_EMITTER_INFO *Info);
  void deleteRSParticleEmitter(const std::string &Name);
  RSParticleEmitter *getRSParticleEmitter(const std::string &Name);

  std::vector<RSParticleEmitter *> *getAllParticleEmitters();

  void startRSParticleEmitter(const std::string &Name);
  void pauseRSParticleEmitter(const std::string &Name);

  inline void lockContainer() { DataLock.lock(); }
  inline void unlockContainer() { DataLock.unlock(); }
};
