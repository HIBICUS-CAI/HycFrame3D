#include "InstanceSystem.h"
#include "SystemExecutive.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "ATransformComponent.h"
#include "UTransformComponent.h"
#include "AMeshComponent.h"
#include "ALightComponent.h"
#include "AParticleComponent.h"
#include "ASpriteComponent.h"
#include "USpriteComponent.h"
#include "UAnimateComponent.h"

InstanceSystem::InstanceSystem(SystemExecutive* _sysExecutive) :
    System("instance-system", _sysExecutive),
    mATransVecPtr(nullptr), mUTransVecPtr(nullptr), mAMeshVecPtr(nullptr),
    mALightVecPtr(nullptr), mAParitcleVecPtr(nullptr), mASpriteVecPtr(nullptr),
    mUSpriteVecPtr(nullptr), mUAnimateVecPtr(nullptr)
{

}

InstanceSystem::~InstanceSystem()
{

}

bool InstanceSystem::Init()
{
#ifdef _DEBUG
    assert(GetSystemExecutive());
#endif // _DEBUG

    mATransVecPtr = (std::vector<ATransformComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::A_TRANSFORM);
    mUTransVecPtr = (std::vector<UTransformComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::U_TRANSFORM);
    mAMeshVecPtr = (std::vector<AMeshComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::A_MESH);
    mALightVecPtr = (std::vector<ALightComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::A_LIGHT);
    mAParitcleVecPtr = (std::vector<AParticleComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::A_PARTICLE);
    mASpriteVecPtr = (std::vector<ASpriteComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::A_SPRITE);
    mUSpriteVecPtr = (std::vector<USpriteComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::U_SPRITE);
    mUAnimateVecPtr = (std::vector<UAnimateComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::U_ANIMATE);

    if (!(mATransVecPtr && mUTransVecPtr && mAMeshVecPtr && mASpriteVecPtr &&
        mALightVecPtr && mAParitcleVecPtr && mUSpriteVecPtr && mUAnimateVecPtr))
    {
        return false;
    }

    return true;
}

void InstanceSystem::Run(Timer& _timer)
{
    for (auto& atc : *mATransVecPtr)
    {
        if (atc.GetCompStatus() == STATUS::ACTIVE) { atc.Update(_timer); }
    }

    for (auto& utc : *mUTransVecPtr)
    {
        if (utc.GetCompStatus() == STATUS::ACTIVE) { utc.Update(_timer); }
    }

    for (auto& amc : *mAMeshVecPtr)
    {
        if (amc.GetCompStatus() == STATUS::ACTIVE) { amc.Update(_timer); }
    }

    for (auto& alc : *mALightVecPtr)
    {
        if (alc.GetCompStatus() == STATUS::ACTIVE) { alc.Update(_timer); }
    }

    for (auto& apc : *mAParitcleVecPtr)
    {
        if (apc.GetCompStatus() == STATUS::ACTIVE) { apc.Update(_timer); }
    }

    for (auto& asc : *mASpriteVecPtr)
    {
        if (asc.GetCompStatus() == STATUS::ACTIVE) { asc.Update(_timer); }
    }

    for (auto& usc : *mUSpriteVecPtr)
    {
        if (usc.GetCompStatus() == STATUS::ACTIVE) { usc.Update(_timer); }
    }

    for (auto& uamc : *mUAnimateVecPtr)
    {
        if (uamc.GetCompStatus() == STATUS::ACTIVE) { uamc.Update(_timer); }
    }
}

void InstanceSystem::Destory()
{

}
