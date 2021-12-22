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

    class ActorObject* GetActorObject(std::string&& _actorName);
    class ActorObject* GetActorObject(std::string& _actorName);
    void AddActorObject(class ActorObject& _newActor);
    void DeleteActorObject(std::string&& _actorName);
    void DeleteActorObject(std::string& _actorName_actorName);
    class UiObject* GetUiObject(std::string&& _uiName);
    class UiObject* GetUiObject(std::string& _uiName);
    void AddUiObject(class UiObject& _newUi);
    void DeleteUiObject(std::string&& _uiName);
    void DeleteUiObject(std::string& _uiName);

    void DeleteAllActor();
    void DeleteAllUi();

private:
    void InitAllNewObjects();
    void DeleteAllDeadObjects();

private:
    SceneNode& mSceneNodeOwner;
    
    //std::vector<ActorObject> mActorObjVector;
    std::unordered_map<std::string, class ActorObject&> mActorObjMap;
    //std::vector<UiObject> mUiObjVector;
    std::unordered_map<std::string, class UiObject&> mUiObjMap;

    //std::vector<ActorObject> mNewActorObjVector;
    //std::vector<UiObject> mNewUiObjVector;
    //std::vector<ActorObject> mDeadActorObjVector;
    //std::vector<UiObject> mDeadUiObjVector;
};
