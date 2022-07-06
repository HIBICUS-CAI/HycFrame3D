#include "AnimationSystem.h"
#include "SystemExecutive.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "AssetsPool.h"
#include "ComponentContainer.h"
#include "AAnimateComponent.h"

AnimationSystem::AnimationSystem(SystemExecutive* _sysExecutive) :
    System("animation-system", _sysExecutive),
    mAAnimateVecPtr(nullptr)
{

}

AnimationSystem::~AnimationSystem()
{

}

bool AnimationSystem::Init()
{
#ifdef _DEBUG
    assert(GetSystemExecutive());
#endif // _DEBUG

    mAAnimateVecPtr = (std::vector<AAnimateComponent>*)GetSystemExecutive()->
        GetSceneManager()->getCurrentSceneNode()->
        getComponentContainer()->getCompVecPtr(COMP_TYPE::A_ANIMATE);

    if (!mAAnimateVecPtr) { return false; }

    return true;
}

void AnimationSystem::Run(Timer& _timer)
{
    for (auto& aanc : *mAAnimateVecPtr)
    {
        if (aanc.getCompStatus() == STATUS::ACTIVE) { aanc.update(_timer); }
    }
}

void AnimationSystem::Destory()
{

}
