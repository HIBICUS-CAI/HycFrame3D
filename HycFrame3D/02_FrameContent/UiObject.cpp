#include "UiObject.h"
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "UiComponent.h"

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
    std::string compName = GetObjectName();

    switch (_compType)
    {
    case COMP_TYPE::U_TRANSFORM: compName += "-transform"; break;
    case COMP_TYPE::U_SPRITE: compName += "-sprite"; break;
    case COMP_TYPE::U_ANIMATE: compName += "-animate"; break;
    case COMP_TYPE::U_TIMER: compName += "-timer"; break;
    case COMP_TYPE::U_INPUT: compName += "-input"; break;
    case COMP_TYPE::U_INTERACT: compName += "-interact"; break;
    case COMP_TYPE::U_BUTTON: compName += "-button"; break;
    case COMP_TYPE::U_AUDIO: compName += "-audio"; break;
    default: break;
    }

    mUiCompMap.insert({ _compType,compName });
}

bool UiObject::Init()
{
    auto compContainer = GetSceneNode().GetComponentContainer();

    for (auto& compInfo : mUiCompMap)
    {
        auto comp = compContainer->GetComponent(compInfo.second);
#ifdef _DEBUG
        assert(comp);
#endif // _DEBUG
        ((UiComponent*)comp)->ResetUiOwner(this);
        if (!comp->Init()) { return false; }
        comp->SetCompStatus(STATUS::ACTIVE);
    }

    SetObjectStatus(STATUS::ACTIVE);

    return true;
}

void UiObject::Destory()
{
    auto compContainer = GetSceneNode().GetComponentContainer();

    for (auto& compInfo : mUiCompMap)
    {
        auto comp = compContainer->GetComponent(compInfo.second);
#ifdef _DEBUG
        assert(comp);
#endif // _DEBUG
        comp->SetCompStatus(STATUS::NEED_DESTORY);
        compContainer->DeleteComponent(compInfo.first, compInfo.second);
    }

    mUiCompMap.clear();
}
