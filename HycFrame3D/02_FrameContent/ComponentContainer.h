#pragma once

#include "Hyc3DCommon.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <queue>

class ComponentContainer
{
public:
    ComponentContainer(class SceneNode& _sceneNode);
    ~ComponentContainer();

    class Component* GetComponent(std::string&& _compName);
    class Component* GetComponent(std::string& _compName);

    void AddComponent(COMP_TYPE _type, class Component& _comp);
    void DeleteComponent(COMP_TYPE _type, std::string&& _compName);
    void DeleteComponent(COMP_TYPE _type, std::string& _compName);

    void DeleteAllComponent();

    void* GetCompVecPtr(COMP_TYPE _type) const;

private:
    const class SceneNode& mSceneNodeOwner;

    std::vector<class ATransformComponent> mATransformCompVector;
    std::vector<class AInputComponent> mAInputCompVector;
    std::vector<class AInteractComponent> mAInteractCompVector;
    std::vector<class ATimerComponent> mATimerCompVector;
    std::vector<class ACollisionComponent> mACollisionCompVector;
    std::vector<class AMeshComponent> mAMeshCompVector;
    std::vector<class ALightComponent> mALightCompVector;
    std::vector<class AAudioComponent> mAAudioCompVector;
    std::vector<class AParticleComponent> mAParticleCompVector;

    std::vector<class UTransformComponent> mUTransformCompVector;
    std::vector<class USpriteComponent> mUSpriteCompVector;
    std::vector<class UAnimateComponent> mUAnimateCompVector;
    std::vector<class UTimerComponent> mUTimerCompVector;
    std::vector<class UInputComponent> mUInputCompVector;
    std::vector<class UInteractComponent> mUInteractCompVector;
    std::vector<class UButtonComponent> mUButtonCompVector;
    std::vector<class UAudioComponent> mUAudioCompVector;

    std::array<std::queue<UINT>, (size_t)COMP_TYPE::SIZE> mInsertFlag;

    std::unordered_map<std::string, class Component*> mCompMap;
};
