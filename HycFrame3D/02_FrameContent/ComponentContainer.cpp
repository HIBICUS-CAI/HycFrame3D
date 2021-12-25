#include "ComponentContainer.h"
#include "SceneNode.h"
#include "ATransformComponent.h"
#include "AInputComponent.h"
#include "AInteractComponent.h"
#include "ATimerComponent.h"
#include "ACollisionComponent.h"
#include "AMeshComponent.h"
#include "ALightComponent.h"
#include "AAudioComponent.h"
#include "AParticleComponent.h"
#include "UTransformComponent.h"
#include "USpriteComponent.h"
#include "UAnimateComponent.h"
#include "UTimerComponent.h"
#include "UInputComponent.h"
#include "UInteractComponent.h"
#include "UButtonComponent.h"
#include "UAudioComponent.h"

ComponentContainer::ComponentContainer(SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode), mCompMap({}),
    mATransformCompVector({}), mAInputCompVector({}),
    mAInteractCompVector({}), mATimerCompVector({}),
    mACollisionCompVector({}), mAMeshCompVector({}),
    mALightCompVector({}), mAAudioCompVector({}), mAParticleCompVector({}),
    mUTransformCompVector({}), mUSpriteCompVector({}),
    mUAnimateCompVector({}), mUTimerCompVector({}),
    mUInputCompVector({}), mUInteractCompVector({}),
    mUButtonCompVector({}), mUAudioCompVector({}),
    mInsertFlag({})
{

}

ComponentContainer::~ComponentContainer()
{

}

Component* ComponentContainer::GetComponent(std::string&& _compName)
{
    if (mCompMap.find(_compName) != mCompMap.end())
    {
        return mCompMap[_compName];
    }
    else
    {
        return nullptr;
    }
}

Component* ComponentContainer::GetComponent(std::string& _compName)
{
    if (mCompMap.find(_compName) != mCompMap.end())
    {
        return mCompMap[_compName];
    }
    else
    {
        return nullptr;
    }
}

void ComponentContainer::AddComponent(COMP_TYPE _type, Component& _comp)
{

}

void ComponentContainer::DeleteComponent(COMP_TYPE _type, std::string&& _compName)
{

}

void ComponentContainer::DeleteComponent(COMP_TYPE _type, std::string& _compName)
{

}

void ComponentContainer::DeleteAllComponent()
{

}

void* ComponentContainer::GetCompVecPtr(COMP_TYPE _type) const
{
    switch (_type)
    {
    case COMP_TYPE::A_TRANSFORM:
        return (void*)&mATransformCompVector;
    case COMP_TYPE::A_INPUT:
        return (void*)&mAInputCompVector;
    case COMP_TYPE::A_INTERACT:
        return (void*)&mAInteractCompVector;
    case COMP_TYPE::A_TIMER:
        return (void*)&mATimerCompVector;
    case COMP_TYPE::A_COLLISION:
        return (void*)&mACollisionCompVector;
    case COMP_TYPE::A_MESH:
        return (void*)&mAMeshCompVector;
    case COMP_TYPE::A_LIGHT:
        return (void*)&mALightCompVector;
    case COMP_TYPE::A_AUDIO:
        return (void*)&mAAudioCompVector;
    case COMP_TYPE::A_PARTICLE:
        return (void*)&mAParticleCompVector;
    case COMP_TYPE::U_TRANSFORM:
        return (void*)&mUTransformCompVector;
    case COMP_TYPE::U_SPRITE:
        return (void*)&mUSpriteCompVector;
    case COMP_TYPE::U_ANIMATE:
        return (void*)&mUAnimateCompVector;
    case COMP_TYPE::U_TIMER:
        return (void*)&mUTimerCompVector;
    case COMP_TYPE::U_INPUT:
        return (void*)&mUInputCompVector;
    case COMP_TYPE::U_INTERACT:
        return (void*)&mUInteractCompVector;
    case COMP_TYPE::U_BUTTON:
        return (void*)&mUButtonCompVector;
    case COMP_TYPE::U_AUDIO:
        return (void*)&mUAudioCompVector;
    default:
        return nullptr;
    }
}
