#pragma once

#include "Hyc3DCommon.h"
#include <string>

class ObjectFactory
{
public:
    ObjectFactory();
    ~ObjectFactory();

    bool StartUp(class SceneManager* _sceneManager);
    void CleanAndStop();

    class SceneNode* CreateSceneNode(std::string _name, std::string _file);
    class ActorObject* CreateActorObject(std::string _name, std::string _file);
    class UiObject* CreateUiObject(std::string _name, std::string _file);

private:
    class SceneManager* mSceneManagerPtr;
};
