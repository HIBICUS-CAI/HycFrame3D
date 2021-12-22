#include "ObjectFactory.h"

ObjectFactory::ObjectFactory() :mSceneManagerPtr(nullptr) {}

ObjectFactory::~ObjectFactory() {}

bool ObjectFactory::StartUp(SceneManager* _sceneManager)
{
    if (!_sceneManager)
    {
        P_LOG(LOG_ERROR, "invalid scene manager pointer\n");
        return false;
    }

    mSceneManagerPtr = _sceneManager;
    return true;
}

void ObjectFactory::CleanAndStop()
{

}

SceneNode* ObjectFactory::CreateSceneNode(std::string _name, std::string _path)
{
    // TEMP--------------------
    return nullptr;
    // TEMP--------------------
}

ActorObject* ObjectFactory::CreateActorObject(std::string _name, std::string _path)
{
    // TEMP--------------------
    return nullptr;
    // TEMP--------------------
}

UiObject* ObjectFactory::CreateUiObject(std::string _name, std::string _path)
{
    // TEMP--------------------
    return nullptr;
    // TEMP--------------------
}
