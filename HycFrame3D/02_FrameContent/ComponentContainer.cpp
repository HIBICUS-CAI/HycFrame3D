#include "ComponentContainer.h"
#include "SceneNode.h"

ComponentContainer::ComponentContainer(SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode),
    /*mCompVector({}), */mCompMap({})
{

}

ComponentContainer::~ComponentContainer()
{

}

Component* ComponentContainer::GetComponent(std::string&& _compName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

Component* ComponentContainer::GetComponent(std::string& _compName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
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
