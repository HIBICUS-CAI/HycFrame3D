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
#include "AAnimateComponent.h"
#include "ASpriteComponent.h"
#include "UTransformComponent.h"
#include "USpriteComponent.h"
#include "UAnimateComponent.h"
#include "UTimerComponent.h"
#include "UInputComponent.h"
#include "UInteractComponent.h"
#include "UButtonComponent.h"
#include "UAudioComponent.h"

ComponentContainer::ComponentContainer(SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode),
    mATransformCompVector({}),
    mAInputCompVector({}),
    mAInteractCompVector({}),
    mATimerCompVector({}),
    mACollisionCompVector({}),
    mAMeshCompVector({}),
    mALightCompVector({}),
    mAAudioCompVector({}),
    mAParticleCompVector({}),
    mAAnimateCompVector({}),
    mASpriteCompVector({}),
    mUTransformCompVector({}),
    mUSpriteCompVector({}),
    mUAnimateCompVector({}),
    mUTimerCompVector({}),
    mUInputCompVector({}),
    mUInteractCompVector({}),
    mUButtonCompVector({}),
    mUAudioCompVector({}),
    mInsertFlag({}),
    mCompMap({})
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
    mAAnimateCompVector.reserve(1024);
    mASpriteCompVector.reserve(1024);
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
        case COMP_TYPE::A_ANIMATE:
            mAAnimateCompVector.emplace_back((AAnimateComponent&)_comp);
            mCompMap.insert(
                { mAAnimateCompVector.back().GetCompName(),
                &(mAAnimateCompVector.back()) });
            break;
        case COMP_TYPE::A_SPRITE:
            mASpriteCompVector.emplace_back((ASpriteComponent&)_comp);
            mCompMap.insert(
                { mASpriteCompVector.back().GetCompName(),
                &(mASpriteCompVector.back()) });
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
            mATransformCompVector[index] = (ATransformComponent&)_comp;
            mCompMap.insert(
                { mATransformCompVector[index].GetCompName(),
                &(mATransformCompVector[index]) });
            break;
        case COMP_TYPE::A_INPUT:
            mAInputCompVector[index] = (AInputComponent&)_comp;
            mCompMap.insert(
                { mAInputCompVector[index].GetCompName(),
                &(mAInputCompVector[index]) });
            break;
        case COMP_TYPE::A_INTERACT:
            mAInteractCompVector[index] = (AInteractComponent&)_comp;
            mCompMap.insert(
                { mAInteractCompVector[index].GetCompName(),
                &(mAInteractCompVector[index]) });
            break;
        case COMP_TYPE::A_TIMER:
            mATimerCompVector[index] = (ATimerComponent&)_comp;
            mCompMap.insert(
                { mATimerCompVector[index].GetCompName(),
                &(mATimerCompVector[index]) });
            break;
        case COMP_TYPE::A_COLLISION:
            mACollisionCompVector[index] = (ACollisionComponent&)_comp;
            mCompMap.insert(
                { mACollisionCompVector[index].GetCompName(),
                &(mACollisionCompVector[index]) });
            break;
        case COMP_TYPE::A_MESH:
            mAMeshCompVector[index] = (AMeshComponent&)_comp;
            mCompMap.insert(
                { mAMeshCompVector[index].GetCompName(),
                &(mAMeshCompVector[index]) });
            break;
        case COMP_TYPE::A_LIGHT:
            mALightCompVector[index] = (ALightComponent&)_comp;
            mCompMap.insert(
                { mALightCompVector[index].GetCompName(),
                &(mALightCompVector[index]) });
            break;
        case COMP_TYPE::A_AUDIO:
            mAAudioCompVector[index] = (AAudioComponent&)_comp;
            mCompMap.insert(
                { mAAudioCompVector[index].GetCompName(),
                &(mAAudioCompVector[index]) });
            break;
        case COMP_TYPE::A_PARTICLE:
            mAParticleCompVector[index] = (AParticleComponent&)_comp;
            mCompMap.insert(
                { mAParticleCompVector[index].GetCompName(),
                &(mAParticleCompVector[index]) });
            break;
        case COMP_TYPE::A_ANIMATE:
            mAAnimateCompVector[index] = (AAnimateComponent&)_comp;
            mCompMap.insert(
                { mAAnimateCompVector[index].GetCompName(),
                &(mAAnimateCompVector[index]) });
            break;
        case COMP_TYPE::A_SPRITE:
            mASpriteCompVector[index] = (ASpriteComponent&)_comp;
            mCompMap.insert(
                { mASpriteCompVector[index].GetCompName(),
                &(mASpriteCompVector[index]) });
            break;
        case COMP_TYPE::U_TRANSFORM:
            mUTransformCompVector[index] = (UTransformComponent&)_comp;
            mCompMap.insert(
                { mUTransformCompVector[index].GetCompName(),
                &(mUTransformCompVector[index]) });
            break;
        case COMP_TYPE::U_SPRITE:
            mUSpriteCompVector[index] = (USpriteComponent&)_comp;
            mCompMap.insert(
                { mUSpriteCompVector[index].GetCompName(),
                &(mUSpriteCompVector[index]) });
            break;
        case COMP_TYPE::U_ANIMATE:
            mUAnimateCompVector[index] = (UAnimateComponent&)_comp;
            mCompMap.insert(
                { mUAnimateCompVector[index].GetCompName(),
                &(mUAnimateCompVector[index]) });
            break;
        case COMP_TYPE::U_TIMER:
            mUTimerCompVector[index] = (UTimerComponent&)_comp;
            mCompMap.insert(
                { mUTimerCompVector[index].GetCompName(),
                &(mUTimerCompVector[index]) });
            break;
        case COMP_TYPE::U_INPUT:
            mUInputCompVector[index] = (UInputComponent&)_comp;
            mCompMap.insert(
                { mUInputCompVector[index].GetCompName(),
                &(mUInputCompVector[index]) });
            break;
        case COMP_TYPE::U_INTERACT:
            mUInteractCompVector[index] = (UInteractComponent&)_comp;
            mCompMap.insert(
                { mUInteractCompVector[index].GetCompName(),
                &(mUInteractCompVector[index]) });
            break;
        case COMP_TYPE::U_BUTTON:
            mUButtonCompVector[index] = (UButtonComponent&)_comp;
            mCompMap.insert(
                { mUButtonCompVector[index].GetCompName(),
                &(mUButtonCompVector[index]) });
            break;
        case COMP_TYPE::U_AUDIO:
            mUAudioCompVector[index] = (UAudioComponent&)_comp;
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
    UINT offset = 0;

    switch (_type)
    {
    case COMP_TYPE::A_TRANSFORM:
    {
        std::vector<ATransformComponent>::size_type voffset =
            (ATransformComponent*)dele - &mATransformCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_INPUT:
    {
        std::vector<AInputComponent>::size_type voffset =
            (AInputComponent*)dele - &mAInputCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_INTERACT:
    {
        std::vector<AInteractComponent>::size_type voffset =
            (AInteractComponent*)dele - &mAInteractCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_TIMER:
    {
        std::vector<ATimerComponent>::size_type voffset =
            (ATimerComponent*)dele - &mATimerCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_COLLISION:
    {
        std::vector<ACollisionComponent>::size_type voffset =
            (ACollisionComponent*)dele - &mACollisionCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_MESH:
    {
        std::vector<AMeshComponent>::size_type voffset =
            (AMeshComponent*)dele - &mAMeshCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_LIGHT:
    {
        std::vector<ALightComponent>::size_type voffset =
            (ALightComponent*)dele - &mALightCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_AUDIO:
    {
        std::vector<AAudioComponent>::size_type voffset =
            (AAudioComponent*)dele - &mAAudioCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_PARTICLE:
    {
        std::vector<AParticleComponent>::size_type voffset =
            (AParticleComponent*)dele - &mAParticleCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_ANIMATE:
    {
        std::vector<AAnimateComponent>::size_type voffset =
            (AAnimateComponent*)dele - &mAAnimateCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_SPRITE:
    {
        std::vector<ASpriteComponent>::size_type voffset =
            (ASpriteComponent*)dele - &mASpriteCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_TRANSFORM:
    {
        std::vector<UTransformComponent>::size_type voffset =
            (UTransformComponent*)dele - &mUTransformCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_SPRITE:
    {
        std::vector<USpriteComponent>::size_type voffset =
            (USpriteComponent*)dele - &mUSpriteCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_ANIMATE:
    {
        std::vector<UAnimateComponent>::size_type voffset =
            (UAnimateComponent*)dele - &mUAnimateCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_TIMER:
    {
        std::vector<UTimerComponent>::size_type voffset =
            (UTimerComponent*)dele - &mUTimerCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_INPUT:
    {
        std::vector<UInputComponent>::size_type voffset =
            (UInputComponent*)dele - &mUInputCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_INTERACT:
    {
        std::vector<UInteractComponent>::size_type voffset =
            (UInteractComponent*)dele - &mUInteractCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_BUTTON:
    {
        std::vector<UButtonComponent>::size_type voffset =
            (UButtonComponent*)dele - &mUButtonCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_AUDIO:
    {
        std::vector<UAudioComponent>::size_type voffset =
            (UAudioComponent*)dele - &mUAudioCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    default:
    {
        bool invalid_comp_to_delete = false;
        assert(invalid_comp_to_delete);
        (void)invalid_comp_to_delete;
        break;
    }
    }

    mCompMap.erase(found);
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
    UINT offset = 0;

    switch (_type)
    {
    case COMP_TYPE::A_TRANSFORM:
    {
        std::vector<ATransformComponent>::size_type voffset =
            (ATransformComponent*)dele - &mATransformCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_INPUT:
    {
        std::vector<AInputComponent>::size_type voffset =
            (AInputComponent*)dele - &mAInputCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_INTERACT:
    {
        std::vector<AInteractComponent>::size_type voffset =
            (AInteractComponent*)dele - &mAInteractCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_TIMER:
    {
        std::vector<ATimerComponent>::size_type voffset =
            (ATimerComponent*)dele - &mATimerCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_COLLISION:
    {
        std::vector<ACollisionComponent>::size_type voffset =
            (ACollisionComponent*)dele - &mACollisionCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_MESH:
    {
        std::vector<AMeshComponent>::size_type voffset =
            (AMeshComponent*)dele - &mAMeshCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_LIGHT:
    {
        std::vector<ALightComponent>::size_type voffset =
            (ALightComponent*)dele - &mALightCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_AUDIO:
    {
        std::vector<AAudioComponent>::size_type voffset =
            (AAudioComponent*)dele - &mAAudioCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_PARTICLE:
    {
        std::vector<AParticleComponent>::size_type voffset =
            (AParticleComponent*)dele - &mAParticleCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_ANIMATE:
    {
        std::vector<AAnimateComponent>::size_type voffset =
            (AAnimateComponent*)dele - &mAAnimateCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::A_SPRITE:
    {
        std::vector<ASpriteComponent>::size_type voffset =
            (ASpriteComponent*)dele - &mASpriteCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_TRANSFORM:
    {
        std::vector<UTransformComponent>::size_type voffset =
            (UTransformComponent*)dele - &mUTransformCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_SPRITE:
    {
        std::vector<USpriteComponent>::size_type voffset =
            (USpriteComponent*)dele - &mUSpriteCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_ANIMATE:
    {
        std::vector<UAnimateComponent>::size_type voffset =
            (UAnimateComponent*)dele - &mUAnimateCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_TIMER:
    {
        std::vector<UTimerComponent>::size_type voffset =
            (UTimerComponent*)dele - &mUTimerCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_INPUT:
    {
        std::vector<UInputComponent>::size_type voffset =
            (UInputComponent*)dele - &mUInputCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_INTERACT:
    {
        std::vector<UInteractComponent>::size_type voffset =
            (UInteractComponent*)dele - &mUInteractCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_BUTTON:
    {
        std::vector<UButtonComponent>::size_type voffset =
            (UButtonComponent*)dele - &mUButtonCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    case COMP_TYPE::U_AUDIO:
    {
        std::vector<UAudioComponent>::size_type voffset =
            (UAudioComponent*)dele - &mUAudioCompVector[0];
        offset = (UINT)voffset;
        break;
    }
    default:
    {
        bool invalid_comp_to_delete = false;
        assert(invalid_comp_to_delete);
        (void)invalid_comp_to_delete;
        break;
    }
    }

    mCompMap.erase(found);
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
    mAAnimateCompVector.clear();
    mASpriteCompVector.clear();
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
    case COMP_TYPE::A_ANIMATE:
        return (void*)&mAAnimateCompVector;
    case COMP_TYPE::A_SPRITE:
        return (void*)&mASpriteCompVector;
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
