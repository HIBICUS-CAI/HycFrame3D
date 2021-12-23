#include "UiObject.h"
#include "SceneNode.h"

UiObject::UiObject(std::string&& _uiName, SceneNode& _sceneNode) :
    Object(_uiName, _sceneNode), mUiCompMap({})
{

}

UiObject::UiObject(std::string& _uiName, SceneNode& _sceneNode) :
    Object(_uiName, _sceneNode), mUiCompMap({})
{

}

UiObject::~UiObject()
{

}

void UiObject::AddUComponent(COMP_TYPE _compType)
{

}

bool UiObject::Init()
{
    // TEMP-----------------------------
    return true;
    // TEMP-----------------------------
}

void UiObject::Destory()
{

}
