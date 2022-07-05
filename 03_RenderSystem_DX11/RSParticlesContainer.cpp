//---------------------------------------------------------------
// File: RSParticlesContainer.cpp
// Proj: RenderSystem_DX11
// Info: 保存并管理所有的粒子发射器
// Date: 2021.11.11
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSParticlesContainer.h"

#include "RSRoot_DX11.h"

#define LOCK EnterCriticalSection(&DataLock)
#define UNLOCK LeaveCriticalSection(&DataLock)

RSParticlesContainer::RSParticlesContainer()
    : RenderSystemRoot(nullptr), ResetFlag(true), ParticleEmitterArray({}),
      ParticleEmitterMap({}), DataLock({}) {}

RSParticlesContainer::~RSParticlesContainer() {}

bool
RSParticlesContainer::startUp(RSRoot_DX11 *RootPtr) {
  if (!RootPtr) {
    return false;
  }
  RenderSystemRoot = RootPtr;

  InitializeCriticalSection(&DataLock);

  ParticleEmitterArray.reserve(MAX_INSTANCE_SIZE);

  return true;
}

void
RSParticlesContainer::cleanAndStop() {
  ParticleEmitterMap.clear();
  ParticleEmitterArray.clear();

  DeleteCriticalSection(&DataLock);
}

bool
RSParticlesContainer::getResetFlg() {
  return ResetFlag;
}

void
RSParticlesContainer::resetRSParticleSystem() {
  RSParticleEmitter::resetEmitterIndex();
  ResetFlag = true;
}

void
RSParticlesContainer::finishResetRSParticleSystem() {
  ResetFlag = false;
}

RSParticleEmitter *
RSParticlesContainer::createRSParticleEmitter(std::string &Name,
                                              PARTICLE_EMITTER_INFO *Info) {
  auto Size = ParticleEmitterArray.size();
  (void)Size;
  RSParticleEmitter *Emitter = new RSParticleEmitter(Info);
  LOCK;
  ParticleEmitterArray.push_back(Emitter);
  ParticleEmitterMap.insert({Name, Emitter});
  UNLOCK;

  return Emitter;
}

void
RSParticlesContainer::deleteRSParticleEmitter(std::string &Name) {
  LOCK;
  auto Found = ParticleEmitterMap.find(Name);
  if (Found != ParticleEmitterMap.end()) {
    RSParticleEmitter *EmitterPtr = Found->second;
    for (auto I = ParticleEmitterArray.begin(), E = ParticleEmitterArray.end();
         I != E; I++) {
      if ((*I) == EmitterPtr) {
        delete EmitterPtr;
        ParticleEmitterArray.erase(I);
        ParticleEmitterMap.erase(Found);
        break;
      }
    }
  }
  UNLOCK;
}

RSParticleEmitter *
RSParticlesContainer::getRSParticleEmitter(std::string &Name) {
  LOCK;
  auto Found = ParticleEmitterMap.find(Name);
  if (Found != ParticleEmitterMap.end()) {
    auto Emitter = Found->second;
    UNLOCK;
    return Emitter;
  } else {
    UNLOCK;
    return nullptr;
  }
}

std::vector<RSParticleEmitter *> *
RSParticlesContainer::getAllParticleEmitters() {
  return &ParticleEmitterArray;
}

void
RSParticlesContainer::startRSParticleEmitter(std::string &Name) {
  LOCK;
  auto Found = ParticleEmitterMap.find(Name);
  if (Found != ParticleEmitterMap.end()) {
    Found->second->startParticleEmitter();
  }
  UNLOCK;
}

void
RSParticlesContainer::pauseRSParticleEmitter(std::string &Name) {
  LOCK;
  auto Found = ParticleEmitterMap.find(Name);
  if (Found != ParticleEmitterMap.end()) {
    Found->second->pauseParticleEmitter();
  }
  UNLOCK;
}
