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
