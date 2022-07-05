//---------------------------------------------------------------
// File: RSParticleEmitter.h
// Proj: RenderSystem_DX11
// Info: 描述并具体执行一个粒子发射器的具体机能
// Date: 2021.11.11
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

class RSParticleEmitter {
private:
  RS_PARTICLE_EMITTER_INFO RSParticleEmitterInfo;
  bool ActiveFlag;
  bool StaticFlag;

public:
  RSParticleEmitter(const PARTICLE_EMITTER_INFO *Info);
  ~RSParticleEmitter();

  void
  resetParticleEmitterInfo(const PARTICLE_EMITTER_INFO *Info);

  RS_PARTICLE_EMITTER_INFO &
  getRSParticleEmitterInfo();

  void
  startParticleEmitter();

  void
  pauseParticleEmitter();

  void
  setEmitterPosition(const DirectX::XMFLOAT3 &Position);

  static void
  resetEmitterIndex();
};
