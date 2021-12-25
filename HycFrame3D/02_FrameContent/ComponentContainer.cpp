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
    mATransformCompVector.reserve(1024);
    mAInputCompVector.reserve(1024);
    mAInteractCompVector.reserve(1024);
    mATimerCompVector.reserve(1024);
    mACollisionCompVector.reserve(1024);
    mAMeshCompVector.reserve(1024);
    mALightCompVector.reserve(1024);
    mAAudioCompVector.reserve(1024);
    mAParticleCompVector.reserve(1024);
    mUTransformCompVector.reserve(1024);
    mUSpriteCompVector.reserve(1024);
    mUAnimateCompVector.reserve(1024);
    mUTimerCompVector.reserve(1024);
    mUInputCompVector.reserve(1024);
    mUInteractCompVector.reserve(1024);
    mUButtonCompVector.reserve(1024);
    mUAudioCompVector.reserve(1024);
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
        P_LOG(LOG_WARNING,
            "cannot find component name : %s\n",
            _compName.c_str());
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
        P_LOG(LOG_WARNING,
            "cannot find component name : %s\n",
            _compName.c_str());
        return nullptr;
    }
}

void ComponentContainer::AddComponent(COMP_TYPE _type, Component& _comp)
{
    if (mInsertFlag[(size_t)_type].empty())
    {
        switch (_type)
        {
        case COMP_TYPE::A_TRANSFORM:
            mATransformCompVector.emplace_back((ATransformComponent&)_comp);
            mCompMap.insert(
                { mATransformCompVector.back().GetCompName(),
                &(mATransformCompVector.back()) });
            break;
        case COMP_TYPE::A_INPUT:
            mAInputCompVector.emplace_back((AInputComponent&)_comp);
            mCompMap.insert(
                { mAInputCompVector.back().GetCompName(),
                &(mAInputCompVector.back()) });
            break;
        case COMP_TYPE::A_INTERACT:
            mAInteractCompVector.emplace_back((AInteractComponent&)_comp);
            mCompMap.insert(
                { mAInteractCompVector.back().GetCompName(),
                &(mAInteractCompVector.back()) });
            break;
        case COMP_TYPE::A_TIMER:
            mATimerCompVector.emplace_back((ATimerComponent&)_comp);
            mCompMap.insert(
                { mATimerCompVector.back().GetCompName(),
                &(mATimerCompVector.back()) });
            break;
        case COMP_TYPE::A_COLLISION:
            mACollisionCompVector.emplace_back((ACollisionComponent&)_comp);
            mCompMap.insert(
                { mACollisionCompVector.back().GetCompName(),
                &(mACollisionCompVector.back()) });
            break;
        case COMP_TYPE::A_MESH:
            mAMeshCompVector.emplace_back((AMeshComponent&)_comp);
            mCompMap.insert(
                { mAMeshCompVector.back().GetCompName(),
                &(mAMeshCompVector.back()) });
            break;
        case COMP_TYPE::A_LIGHT:
            mALightCompVector.emplace_back((ALightComponent&)_comp);
            mCompMap.insert(
                { mALightCompVector.back().GetCompName(),
                &(mALightCompVector.back()) });
            break;
        case COMP_TYPE::A_AUDIO:
            mAAudioCompVector.emplace_back((AAudioComponent&)_comp);
            mCompMap.insert(
                { mAAudioCompVector.back().GetCompName(),
                &(mAAudioCompVector.back()) });
            break;
        case COMP_TYPE::A_PARTICLE:
            mAParticleCompVector.emplace_back((AParticleComponent&)_comp);
            mCompMap.insert(
                { mAParticleCompVector.back().GetCompName(),
                &(mAParticleCompVector.back()) });
            break;
        case COMP_TYPE::U_TRANSFORM:
            mUTransformCompVector.emplace_back((UTransformComponent&)_comp);
            mCompMap.insert(
                { mUTransformCompVector.back().GetCompName(),
                &(mUTransformCompVector.back()) });
            break;
        case COMP_TYPE::U_SPRITE:
            mUSpriteCompVector.emplace_back((USpriteComponent&)_comp);
            mCompMap.insert(
                { mUSpriteCompVector.back().GetCompName(),
                &(mUSpriteCompVector.back()) });
            break;
        case COMP_TYPE::U_ANIMATE:
            mUAnimateCompVector.emplace_back((UAnimateComponent&)_comp);
            mCompMap.insert(
                { mUAnimateCompVector.back().GetCompName(),
                &(mUAnimateCompVector.back()) });
            break;
        case COMP_TYPE::U_TIMER:
            mUTimerCompVector.emplace_back((UTimerComponent&)_comp);
            mCompMap.insert(
                { mUTimerCompVector.back().GetCompName(),
                &(mUTimerCompVector.back()) });
            break;
        case COMP_TYPE::U_INPUT:
            mUInputCompVector.emplace_back((UInputComponent&)_comp);
            mCompMap.insert(
                { mUInputCompVector.back().GetCompName(),
                &(mUInputCompVector.back()) });
            break;
        case COMP_TYPE::U_INTERACT:
            mUInteractCompVector.emplace_back((UInteractComponent&)_comp);
            mCompMap.insert(
                { mUInteractCompVector.back().GetCompName(),
                &(mUInteractCompVector.back()) });
            break;
        case COMP_TYPE::U_BUTTON:
            mUButtonCompVector.emplace_back((UButtonComponent&)_comp);
            mCompMap.insert(
                { mUButtonCompVector.back().GetCompName(),
                &(mUButtonCompVector.back()) });
            break;
        case COMP_TYPE::U_AUDIO:
            mUAudioCompVector.emplace_back((UAudioComponent&)_comp);
            mCompMap.insert(
                { mUAudioCompVector.back().GetCompName(),
                &(mUAudioCompVector.back()) });
            break;
        default:
            break;
        }
    }
    else
    {
        UINT index = mInsertFlag[(size_t)_type].front();
        mInsertFlag[(size_t)_type].pop();

        switch (_type)
        {
        case COMP_TYPE::A_TRANSFORM:
            memcpy_s(
                &mATransformCompVector[index],
                sizeof(mATransformCompVector[index]),
                &(ATransformComponent&)_comp,
                sizeof((ATransformComponent&)_comp));
            mCompMap.insert(
                { mATransformCompVector[index].GetCompName(),
                &(mATransformCompVector[index]) });
            break;
        case COMP_TYPE::A_INPUT:
            memcpy_s(
                &mAInputCompVector[index],
                sizeof(mAInputCompVector[index]),
                &(AInputComponent&)_comp,
                sizeof((AInputComponent&)_comp));
            mCompMap.insert(
                { mAInputCompVector[index].GetCompName(),
                &(mAInputCompVector[index]) });
            break;
        case COMP_TYPE::A_INTERACT:
            memcpy_s(
                &mAInteractCompVector[index],
                sizeof(mAInteractCompVector[index]),
                &(AInteractComponent&)_comp,
                sizeof((AInteractComponent&)_comp));
            mCompMap.insert(
                { mAInteractCompVector[index].GetCompName(),
                &(mAInteractCompVector[index]) });
            break;
        case COMP_TYPE::A_TIMER:
            memcpy_s(
                &mATimerCompVector[index],
                sizeof(mATimerCompVector[index]),
                &(ATimerComponent&)_comp,
                sizeof((ATimerComponent&)_comp));
            mCompMap.insert(
                { mATimerCompVector[index].GetCompName(),
                &(mATimerCompVector[index]) });
            break;
        case COMP_TYPE::A_COLLISION:
            memcpy_s(
                &mACollisionCompVector[index],
                sizeof(mACollisionCompVector[index]),
                &(ACollisionComponent&)_comp,
                sizeof((ACollisionComponent&)_comp));
            mCompMap.insert(
                { mACollisionCompVector[index].GetCompName(),
                &(mACollisionCompVector[index]) });
            break;
        case COMP_TYPE::A_MESH:
            memcpy_s(
                &mAMeshCompVector[index],
                sizeof(mAMeshCompVector[index]),
                &(AMeshComponent&)_comp,
                sizeof((AMeshComponent&)_comp));
            mCompMap.insert(
                { mAMeshCompVector[index].GetCompName(),
                &(mAMeshCompVector[index]) });
            break;
        case COMP_TYPE::A_LIGHT:
            memcpy_s(
                &mALightCompVector[index],
                sizeof(mALightCompVector[index]),
                &(ALightComponent&)_comp,
                sizeof((ALightComponent&)_comp));
            mCompMap.insert(
                { mALightCompVector[index].GetCompName(),
                &(mALightCompVector[index]) });
            break;
        case COMP_TYPE::A_AUDIO:
            memcpy_s(
                &mAAudioCompVector[index],
                sizeof(mAAudioCompVector[index]),
                &(AAudioComponent&)_comp,
                sizeof((AAudioComponent&)_comp));
            mCompMap.insert(
                { mAAudioCompVector[index].GetCompName(),
                &(mAAudioCompVector[index]) });
            break;
        case COMP_TYPE::A_PARTICLE:
            memcpy_s(
                &mAParticleCompVector[index],
                sizeof(mAParticleCompVector[index]),
                &(AParticleComponent&)_comp,
                sizeof((AParticleComponent&)_comp));
            mCompMap.insert(
                { mAParticleCompVector[index].GetCompName(),
                &(mAParticleCompVector[index]) });
            break;
        case COMP_TYPE::U_TRANSFORM:
            memcpy_s(
                &mUTransformCompVector[index],
                sizeof(mUTransformCompVector[index]),
                &(UTransformComponent&)_comp,
                sizeof((UTransformComponent&)_comp));
            mCompMap.insert(
                { mUTransformCompVector[index].GetCompName(),
                &(mUTransformCompVector[index]) });
            break;
        case COMP_TYPE::U_SPRITE:
            memcpy_s(
                &mUSpriteCompVector[index],
                sizeof(mUSpriteCompVector[index]),
                &(USpriteComponent&)_comp,
                sizeof((USpriteComponent&)_comp));
            mCompMap.insert(
                { mUSpriteCompVector[index].GetCompName(),
                &(mUSpriteCompVector[index]) });
            break;
        case COMP_TYPE::U_ANIMATE:
            memcpy_s(
                &mUAnimateCompVector[index],
                sizeof(mUAnimateCompVector[index]),
                &(UAnimateComponent&)_comp,
                sizeof((UAnimateComponent&)_comp));
            mCompMap.insert(
                { mUAnimateCompVector[index].GetCompName(),
                &(mUAnimateCompVector[index]) });
            break;
        case COMP_TYPE::U_TIMER:
            memcpy_s(
                &mUTimerCompVector[index],
                sizeof(mUTimerCompVector[index]),
                &(UTimerComponent&)_comp,
                sizeof((UTimerComponent&)_comp));
            mCompMap.insert(
                { mUTimerCompVector[index].GetCompName(),
                &(mUTimerCompVector[index]) });
            break;
        case COMP_TYPE::U_INPUT:
            memcpy_s(
                &mUInputCompVector[index],
                sizeof(mUInputCompVector[index]),
                &(UInputComponent&)_comp,
                sizeof((UInputComponent&)_comp));
            mCompMap.insert(
                { mUInputCompVector[index].GetCompName(),
                &(mUInputCompVector[index]) });
            break;
        case COMP_TYPE::U_INTERACT:
            memcpy_s(
                &mUInteractCompVector[index],
                sizeof(mUInteractCompVector[index]),
                &(UInteractComponent&)_comp,
                sizeof((UInteractComponent&)_comp));
            mCompMap.insert(
                { mUInteractCompVector[index].GetCompName(),
                &(mUInteractCompVector[index]) });
            break;
        case COMP_TYPE::U_BUTTON:
            memcpy_s(
                &mUButtonCompVector[index],
                sizeof(mUButtonCompVector[index]),
                &(UButtonComponent&)_comp,
                sizeof((UButtonComponent&)_comp));
            mCompMap.insert(
                { mUButtonCompVector[index].GetCompName(),
                &(mUButtonCompVector[index]) });
            break;
        case COMP_TYPE::U_AUDIO:
            memcpy_s(
                &mUAudioCompVector[index],
                sizeof(mUAudioCompVector[index]),
                &(UAudioComponent&)_comp,
                sizeof((UAudioComponent&)_comp));
            mCompMap.insert(
                { mUAudioCompVector[index].GetCompName(),
                &(mUAudioCompVector[index]) });
            break;
        default:
            break;
        }
    }
}

void ComponentContainer::DeleteComponent(COMP_TYPE _type, std::string&& _compName)
{
    auto found = mCompMap.find(_compName);
    if (found == mCompMap.end())
    {
        P_LOG(LOG_WARNING,
            "cannot find component name : %s\n",
            _compName.c_str());
        return;
    }

    Component* dele = found->second;
    Component* start = nullptr;

    switch (_type)
    {
    case COMP_TYPE::A_TRANSFORM:
        start = (Component*)&mATransformCompVector[0];
        break;
    case COMP_TYPE::A_INPUT:
        start = (Component*)&mAInputCompVector[0];
        break;
    case COMP_TYPE::A_INTERACT:
        start = (Component*)&mAInteractCompVector[0];
        break;
    case COMP_TYPE::A_TIMER:
        start = (Component*)&mATimerCompVector[0];
        break;
    case COMP_TYPE::A_COLLISION:
        start = (Component*)&mACollisionCompVector[0];
        break;
    case COMP_TYPE::A_MESH:
        start = (Component*)&mAMeshCompVector[0];
        break;
    case COMP_TYPE::A_LIGHT:
        start = (Component*)&mALightCompVector[0];
        break;
    case COMP_TYPE::A_AUDIO:
        start = (Component*)&mAAudioCompVector[0];
        break;
    case COMP_TYPE::A_PARTICLE:
        start = (Component*)&mAParticleCompVector[0];
        break;
    case COMP_TYPE::U_TRANSFORM:
        start = (Component*)&mUTransformCompVector[0];
        break;
    case COMP_TYPE::U_SPRITE:
        start = (Component*)&mUSpriteCompVector[0];
        break;
    case COMP_TYPE::U_ANIMATE:
        start = (Component*)&mUAnimateCompVector[0];
        break;
    case COMP_TYPE::U_TIMER:
        start = (Component*)&mUTimerCompVector[0];
        break;
    case COMP_TYPE::U_INPUT:
        start = (Component*)&mUInputCompVector[0];
        break;
    case COMP_TYPE::U_INTERACT:
        start = (Component*)&mUInteractCompVector[0];
        break;
    case COMP_TYPE::U_BUTTON:
        start = (Component*)&mUButtonCompVector[0];
        break;
    case COMP_TYPE::U_AUDIO:
        start = (Component*)&mUAudioCompVector[0];
        break;
    default:
        assert(start);
        break;
    }

    mCompMap.erase(found);
    UINT offset = (UINT)(dele - start);
    mInsertFlag[(size_t)_type].push(offset);
}

void ComponentContainer::DeleteComponent(COMP_TYPE _type, std::string& _compName)
{
    auto found = mCompMap.find(_compName);
    if (found == mCompMap.end())
    {
        P_LOG(LOG_WARNING,
            "cannot find component name : %s\n",
            _compName.c_str());
        return;
    }

    Component* dele = found->second;
    Component* start = nullptr;

    switch (_type)
    {
    case COMP_TYPE::A_TRANSFORM:
        start = (Component*)&mATransformCompVector[0];
        break;
    case COMP_TYPE::A_INPUT:
        start = (Component*)&mAInputCompVector[0];
        break;
    case COMP_TYPE::A_INTERACT:
        start = (Component*)&mAInteractCompVector[0];
        break;
    case COMP_TYPE::A_TIMER:
        start = (Component*)&mATimerCompVector[0];
        break;
    case COMP_TYPE::A_COLLISION:
        start = (Component*)&mACollisionCompVector[0];
        break;
    case COMP_TYPE::A_MESH:
        start = (Component*)&mAMeshCompVector[0];
        break;
    case COMP_TYPE::A_LIGHT:
        start = (Component*)&mALightCompVector[0];
        break;
    case COMP_TYPE::A_AUDIO:
        start = (Component*)&mAAudioCompVector[0];
        break;
    case COMP_TYPE::A_PARTICLE:
        start = (Component*)&mAParticleCompVector[0];
        break;
    case COMP_TYPE::U_TRANSFORM:
        start = (Component*)&mUTransformCompVector[0];
        break;
    case COMP_TYPE::U_SPRITE:
        start = (Component*)&mUSpriteCompVector[0];
        break;
    case COMP_TYPE::U_ANIMATE:
        start = (Component*)&mUAnimateCompVector[0];
        break;
    case COMP_TYPE::U_TIMER:
        start = (Component*)&mUTimerCompVector[0];
        break;
    case COMP_TYPE::U_INPUT:
        start = (Component*)&mUInputCompVector[0];
        break;
    case COMP_TYPE::U_INTERACT:
        start = (Component*)&mUInteractCompVector[0];
        break;
    case COMP_TYPE::U_BUTTON:
        start = (Component*)&mUButtonCompVector[0];
        break;
    case COMP_TYPE::U_AUDIO:
        start = (Component*)&mUAudioCompVector[0];
        break;
    default:
        assert(start);
        break;
    }

    mCompMap.erase(found);
    UINT offset = (UINT)(dele - start);
    mInsertFlag[(size_t)_type].push(offset);
}

void ComponentContainer::DeleteAllComponent()
{
    mATransformCompVector.clear();
    mAInputCompVector.clear();
    mAInteractCompVector.clear();
    mATimerCompVector.clear();
    mACollisionCompVector.clear();
    mAMeshCompVector.clear();
    mALightCompVector.clear();
    mAAudioCompVector.clear();
    mAParticleCompVector.clear();
    mUTransformCompVector.clear();
    mUSpriteCompVector.clear();
    mUAnimateCompVector.clear();
    mUTimerCompVector.clear();
    mUInputCompVector.clear();
    mUInteractCompVector.clear();
    mUButtonCompVector.clear();
    mUAudioCompVector.clear();
    mCompMap.clear();
    for (auto& flgArray : mInsertFlag)
    {
        while (!flgArray.empty()) { flgArray.pop(); }
    }
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
