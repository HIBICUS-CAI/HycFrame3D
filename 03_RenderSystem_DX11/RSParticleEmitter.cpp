//---------------------------------------------------------------
// File: RSParticleEmitter.cpp
// Proj: RenderSystem_DX11
// Info: 描述并具体执行一个粒子发射器的具体机能
// Date: 2021.11.11
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSParticleEmitter.h"
#include <assert.h>

static UINT g_ParticleEmitterCounter = 0;

RSParticleEmitter::RSParticleEmitter(PARTICLE_EMITTER_INFO* _info)
    : mRSParticleEmitterInfo({}),
    mActiveFlg(false), mStaticFlg(false)
{
    ResetParticleEmitterInfo(_info);
    (void)mStaticFlg;
}

RSParticleEmitter::~RSParticleEmitter()
{

}

void RSParticleEmitter::ResetParticleEmitterInfo(
    PARTICLE_EMITTER_INFO* _info)
{
    if (!_info)
    {
        bool didnt_pass_a_valid_info_to_particle_reset = false;
        assert(didnt_pass_a_valid_info_to_particle_reset);
        (void)didnt_pass_a_valid_info_to_particle_reset;
        return;
    }

    mRSParticleEmitterInfo.EmitterIndex =
        g_ParticleEmitterCounter++;
    mRSParticleEmitterInfo.EmitNumPerSecond =
        _info->EmitNumPerSecond;
    mRSParticleEmitterInfo.EmitSize = 0;
    mRSParticleEmitterInfo.Accumulation = 0.f;
    mRSParticleEmitterInfo.Position = _info->Position;
    mRSParticleEmitterInfo.Velocity = _info->Velocity;
    mRSParticleEmitterInfo.PosVariance = _info->PosVariance;
    mRSParticleEmitterInfo.VelVariance = _info->VelVariance;
    mRSParticleEmitterInfo.Acceleration = _info->Acceleration;
    mRSParticleEmitterInfo.ParticleMass = _info->ParticleMass;
    mRSParticleEmitterInfo.LifeSpan = _info->LifeSpan;
    mRSParticleEmitterInfo.StartSize =
        _info->StartSize;
    mRSParticleEmitterInfo.EndSize =
        _info->EndSize;
    mRSParticleEmitterInfo.StartColor =
        _info->StartColor;
    mRSParticleEmitterInfo.EndColor =
        _info->EndColor;
    mRSParticleEmitterInfo.TextureID =
        (UINT)(_info->TextureID);
    mRSParticleEmitterInfo.StreakFlag =
        (_info->StreakFlag ? 1 : 0);
    mRSParticleEmitterInfo.MiscFlag = 0;
}

RS_PARTICLE_EMITTER_INFO& RSParticleEmitter::GetRSParticleEmitterInfo()
{
    return mRSParticleEmitterInfo;
}

void RSParticleEmitter::StartParticleEmitter()
{
    mActiveFlg = true;
}

void RSParticleEmitter::PauseParticleEmitter()
{
    mActiveFlg = false;
}

void RSParticleEmitter::SetEmitterPosition(DirectX::XMFLOAT3& _position)
{
    mRSParticleEmitterInfo.Position = _position;
}

void RSParticleEmitter::SetEmitterPosition(DirectX::XMFLOAT3&& _position)
{
    mRSParticleEmitterInfo.Position = _position;
}

void RSParticleEmitter::ResetEmitterIndex()
{
    g_ParticleEmitterCounter = 0;
}
