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

InstanceSystem::InstanceSystem(SystemExecutive* _sysExecutive) :
    System("instance-system", _sysExecutive),
    mATransVecPtr(nullptr), mUTransVecPtr(nullptr), mAMeshVecPtr(nullptr),
    mALightVecPtr(nullptr), mAParitcleVecPtr(nullptr)
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

    if (!(mATransVecPtr && mUTransVecPtr && mAMeshVecPtr &&
        mALightVecPtr && mAParitcleVecPtr))
    {
        return false;
    }

    return true;
}

void InstanceSystem::Run(Timer& _timer)
{
    for (auto& atc : *mATransVecPtr)
    {
        atc.Update(_timer);
    }

    for (auto& amc : *mAMeshVecPtr)
    {
        amc.Update(_timer);
    }

    for (auto& alc : *mALightVecPtr)
    {
        alc.Update(_timer);
    }

    for (auto& apc : *mAParitcleVecPtr)
    {
        apc.Update(_timer);
    }
}

void InstanceSystem::Destory()
{

}
