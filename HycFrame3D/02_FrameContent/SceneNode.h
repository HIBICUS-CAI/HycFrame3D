#pragma once

#include "Hyc3DCommon.h"
#include <string>
#include <DirectXMath.h>

struct CAMERA_AND_AMBIENT
{
    class RSCamera* mRSCameraPtr = nullptr;
    DirectX::XMFLOAT4 mAmbientColor = {};
};

class SceneNode
{
public:
    SceneNode(std::string&& _sceneName, class SceneManager* _sceneManager);
    SceneNode(std::string& _sceneName, class SceneManager* _sceneManager);
    ~SceneNode();

    void ReleaseScene();

    const std::string& GetSceneNodeName() const;
    class SceneManager* GetSceneManager() const;

    void SetCurrentAmbient(DirectX::XMFLOAT4&& _ambientColor);
    void SetCurrentAmbient(DirectX::XMFLOAT4& _ambientColor);
    DirectX::XMFLOAT4& GetCurrentAmbient();

    class RSCamera* GetMainCamera();

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

    class AssetsPool* GetAssetsPool() const;
    class PhysicsWorld* GetPhysicsWorld() const;

private:
    const std::string mSceneName;
    class SceneManager* mSceneManagerPtr;
    class ObjectContainer* mObjContainerPtr;
    class ComponentContainer* mCompContainerPtr;
    class AssetsPool* mAssetsPoolPtr;
    class PhysicsWorld* mPhysicsWorldPtr;
    CAMERA_AND_AMBIENT mCameraAmbientInfo;
};