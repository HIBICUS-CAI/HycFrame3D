#pragma once

#include "Hyc3DCommon.h"
#include <string>
#include <vector>
#include <unordered_map>

class ObjectContainer
{
public:
    ObjectContainer(class SceneNode& _sceneNode);
    ~ObjectContainer();

    class ActorObject* GetActorObject(const std::string& _actorName);
    void AddActorObject(class ActorObject& _newActor);
    void DeleteActorObject(std::string&& _actorName);
    void DeleteActorObject(std::string& _actorName);
    class UiObject* GetUiObject(const std::string& _uiName);
    void AddUiObject(class UiObject& _newUi);
    void DeleteUiObject(std::string&& _uiName);
    void DeleteUiObject(std::string& _uiName);

    void DeleteAllActor();
    void DeleteAllUi();

    void InitAllNewObjects();
    void DeleteAllDeadObjects();

private:
    SceneNode& mSceneNodeOwner;
    
    std::unordered_map<std::string, class ActorObject> mActorObjMap;
    std::unordered_map<std::string, class UiObject> mUiObjMap;

    std::vector<class ActorObject> mNewActorObjVector;
    std::vector<class UiObject> mNewUiObjVector;
    std::vector<class ActorObject> mDeadActorObjVector;
    std::vector<class UiObject> mDeadUiObjVector;
};
