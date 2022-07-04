#pragma once

#include "Hyc3DCommon.h"
#include <TextUtility.h>
#include <string>
#include <unordered_map>
#include "AInputComponent.h"
#include "UInputComponent.h"
#include "AInteractComponent.h"
#include "UInteractComponent.h"

class ObjectFactory
{
public:
    ObjectFactory();
    ~ObjectFactory();

    bool StartUp(class SceneManager* _sceneManager);
    void CleanAndStop();

    class SceneNode* CreateSceneNode(std::string _name, std::string _file);

    std::unordered_map<std::string, ActorInputProcessFuncType>*
        GetAInputMapPtr() { return &mActorInputFuncPtrMap; }
    std::unordered_map<std::string, ActorInteractInitFuncType>*
        GetAInitMapPtr() { return &mActorInteractInitFuncPtrMap; }
    std::unordered_map<std::string, ActorInteractUpdateFuncType>*
        GetAUpdateMapPtr() { return &mActorInteractUpdateFuncPtrMap; }
    std::unordered_map<std::string, ActorInteractDestoryFuncType>*
        GetADestoryMapPtr() { return &mActorInteractDestoryFuncPtrMap; }
    std::unordered_map<std::string, UiInputProcessFuncType>*
        GetUInputMapPtr() { return &mUiInputFuncPtrMap; }
    std::unordered_map<std::string, UiInteractInitFuncType>*
        GetUInitMapPtr() { return &mUiInteractInitFuncPtrMap; }
    std::unordered_map<std::string, UiInteractUpdateFuncType>*
        GetUUpdateMapPtr() { return &mUiInteractUpdateFuncPtrMap; }
    std::unordered_map<std::string, UiInteractDestoryFuncType>*
        GetUDestoryMapPtr() { return &mUiInteractDestoryFuncPtrMap; }

private:
    void CreateSceneAssets(class SceneNode* _node, hyc::text::JsonFile& _json);

    void CreateActorObject(class SceneNode* _node, hyc::text::JsonFile& _json,
        std::string _jsonPath);
    void CreateUiObject(class SceneNode* _node, hyc::text::JsonFile& _json,
        std::string _jsonPath);

    void CreateActorComp(class SceneNode* _node, class ActorObject* _actor,
        hyc::text::JsonFile& _json, std::string _jsonPath);
    void CreateUiComp(class SceneNode* _node, class UiObject* _ui,
        hyc::text::JsonFile& _json, std::string _jsonPath);

private:
    class SceneManager* mSceneManagerPtr;

    std::unordered_map<std::string, ActorInputProcessFuncType>
        mActorInputFuncPtrMap;
    std::unordered_map<std::string, ActorInteractInitFuncType>
        mActorInteractInitFuncPtrMap;
    std::unordered_map<std::string, ActorInteractUpdateFuncType>
        mActorInteractUpdateFuncPtrMap;
    std::unordered_map<std::string, ActorInteractDestoryFuncType>
        mActorInteractDestoryFuncPtrMap;

    std::unordered_map<std::string, UiInputProcessFuncType>
        mUiInputFuncPtrMap;
    std::unordered_map<std::string, UiInteractInitFuncType>
        mUiInteractInitFuncPtrMap;
    std::unordered_map<std::string, UiInteractUpdateFuncType>
        mUiInteractUpdateFuncPtrMap;
    std::unordered_map<std::string, UiInteractDestoryFuncType>
        mUiInteractDestoryFuncPtrMap;
};
