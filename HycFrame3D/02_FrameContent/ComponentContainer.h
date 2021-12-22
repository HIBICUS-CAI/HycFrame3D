#pragma once

#include "Hyc3DCommon.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <array>

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

private:
    const class SceneNode& mSceneNodeOwner;
    //std::array<std::vector<Component>, (size_t)COMP_TYPE::SIZE> mCompVector;
    std::unordered_map<std::string, class Component*> mCompMap;
};
