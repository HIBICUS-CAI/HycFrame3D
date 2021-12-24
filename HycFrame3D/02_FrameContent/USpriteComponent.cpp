#include "USpriteComponent.h"
#include "UiObject.h"

USpriteComponent::USpriteComponent(std::string&& _compName,
    UiObject& _uiOwner) :
    UiComponent(_compName, _uiOwner), mMeshesName({}), mInstancesIndex({})
{

}

USpriteComponent::USpriteComponent(std::string& _compName,
    UiObject& _uiOwner) :
    UiComponent(_compName, _uiOwner), mMeshesName({}), mInstancesIndex({})
{

}

USpriteComponent::~USpriteComponent()
{

}

bool USpriteComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void USpriteComponent::Update(Timer& _timer)
{

}

void USpriteComponent::Destory()
{

}

bool USpriteComponent::BindInstanceToAssetsPool(std::string&& _meshName)
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

bool USpriteComponent::BindInstanceToAssetsPool(std::string& _meshName)
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void USpriteComponent::SyncTransformDataToInstance()
{

}
