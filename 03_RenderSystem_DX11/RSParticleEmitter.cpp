//---------------------------------------------------------------
// File: RSParticleEmitter.cpp
// Proj: RenderSystem_DX11
// Info: 描述并具体执行一个粒子发射器的具体机能
// Date: 2021.11.11
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSParticleEmitter.h"

#include <cassert>

static UINT G_ParticleEmitterCounter = 0;

RSParticleEmitter::RSParticleEmitter(const PARTICLE_EMITTER_INFO *Info)
    : RSParticleEmitterInfo({}), ActiveFlag(false), StaticFlag(false) {
  resetParticleEmitterInfo(Info);
  (void)StaticFlag;
}

RSParticleEmitter::~RSParticleEmitter() {}

void RSParticleEmitter::resetParticleEmitterInfo(
    const PARTICLE_EMITTER_INFO *Info) {
  if (!Info) {
    assert(false && "didnt pass a valid info to particle reset");
    return;
  }

  RSParticleEmitterInfo.EmitterIndex = G_ParticleEmitterCounter++;
  RSParticleEmitterInfo.EmitNumPerSecond = Info->EmitNumPerSecond;
  RSParticleEmitterInfo.EmitSize = 0;
  RSParticleEmitterInfo.Accumulation = 0.f;
  RSParticleEmitterInfo.Position = Info->Position;
  RSParticleEmitterInfo.Velocity = Info->Velocity;
  RSParticleEmitterInfo.PosVariance = Info->PosVariance;
  RSParticleEmitterInfo.VelVariance = Info->VelVariance;
  RSParticleEmitterInfo.Acceleration = Info->Acceleration;
  RSParticleEmitterInfo.ParticleMass = Info->ParticleMass;
  RSParticleEmitterInfo.LifeSpan = Info->LifeSpan;
  RSParticleEmitterInfo.StartSize = Info->StartSize;
  RSParticleEmitterInfo.EndSize = Info->EndSize;
  RSParticleEmitterInfo.StartColor = Info->StartColor;
  RSParticleEmitterInfo.EndColor = Info->EndColor;
  RSParticleEmitterInfo.TextureID = static_cast<UINT>(Info->TextureID);
  RSParticleEmitterInfo.StreakFlag = (Info->StreakFlag ? 1 : 0);
  RSParticleEmitterInfo.MiscFlag = 0;
}

RS_PARTICLE_EMITTER_INFO &RSParticleEmitter::getRSParticleEmitterInfo() {
  return RSParticleEmitterInfo;
}

void RSParticleEmitter::startParticleEmitter() { ActiveFlag = true; }

void RSParticleEmitter::pauseParticleEmitter() { ActiveFlag = false; }

void RSParticleEmitter::setEmitterPosition(const DirectX::XMFLOAT3 &Position) {
  RSParticleEmitterInfo.Position = Position;
}

void RSParticleEmitter::resetEmitterIndex() { G_ParticleEmitterCounter = 0; }
