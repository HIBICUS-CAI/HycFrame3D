#include "SceneNode.h"
#include <vector>
#include "RSCamerasContainer.h"
#include "RSCamera.h"
#include "RSRoot_DX11.h"
#include "ObjectContainer.h"
#include "ComponentContainer.h"
#include "AssetsPool.h"
#include "PhysicsWorld.h"

SceneNode::SceneNode(std::string&& _sceneName, SceneManager* _sceneManager) :
    mSceneName(_sceneName), mSceneManagerPtr(_sceneManager),
    mObjContainerPtr(nullptr), mCompContainerPtr(nullptr),
    mAssetsPoolPtr(nullptr), mPhysicsWorldPtr(nullptr),
    mCameraAmbientInfo({})
{
    mObjContainerPtr = new ObjectContainer(*this);
    mCompContainerPtr = new ComponentContainer(*this);
    mAssetsPoolPtr = new AssetsPool(*this);
    mPhysicsWorldPtr = new PhysicsWorld(*this);
    if (_sceneName != "temp-loading-scene")
    {
        std::string cam = "temp-cam";
        mCameraAmbientInfo.mRSCameraPtr = GetRSRoot_DX11_Singleton()->
            CamerasContainer()->GetRSCamera(cam);
        bool new_scene_fail = mObjContainerPtr && mCompContainerPtr &&
            mAssetsPoolPtr && mPhysicsWorldPtr && mCameraAmbientInfo.mRSCameraPtr;
        assert(new_scene_fail);
    }
}

SceneNode::SceneNode(std::string& _sceneName, SceneManager* _sceneManager) :
    mSceneName(_sceneName), mSceneManagerPtr(_sceneManager),
    mObjContainerPtr(nullptr), mCompContainerPtr(nullptr),
    mAssetsPoolPtr(nullptr), mPhysicsWorldPtr(nullptr),
    mCameraAmbientInfo({})
{
    mObjContainerPtr = new ObjectContainer(*this);
    mCompContainerPtr = new ComponentContainer(*this);
    mAssetsPoolPtr = new AssetsPool(*this);
    mPhysicsWorldPtr = new PhysicsWorld(*this);
    if (_sceneName != "temp-loading-scene")
    {
        std::string cam = "temp-cam";
        mCameraAmbientInfo.mRSCameraPtr = GetRSRoot_DX11_Singleton()->
            CamerasContainer()->GetRSCamera(cam);
        bool new_scene_fail = mObjContainerPtr && mCompContainerPtr &&
            mAssetsPoolPtr && mPhysicsWorldPtr && mCameraAmbientInfo.mRSCameraPtr;
        assert(new_scene_fail);
    }
}

SceneNode::~SceneNode()
{
    delete mPhysicsWorldPtr;
    delete mAssetsPoolPtr;
    delete mCompContainerPtr;
    delete mObjContainerPtr;
}

void SceneNode::ReleaseScene()
{
    mObjContainerPtr->DeleteAllActor();
    mObjContainerPtr->DeleteAllUi();
    mObjContainerPtr->DeleteAllDeadObjects();
    mCompContainerPtr->DeleteAllComponent();
    mPhysicsWorldPtr->DeletePhysicsWorld();
    mAssetsPoolPtr->DeleteAllAssets();
}

const std::string& SceneNode::GetSceneNodeName() const
{
    return mSceneName;
}

SceneManager* SceneNode::GetSceneManager() const
{
    return mSceneManagerPtr;
}

void SceneNode::SetCurrentAmbient(DirectX::XMFLOAT4&& _ambientColor)
{
    mCameraAmbientInfo.mAmbientColor = _ambientColor;
}

void SceneNode::SetCurrentAmbient(DirectX::XMFLOAT4& _ambientColor)
{
    mCameraAmbientInfo.mAmbientColor = _ambientColor;
}

DirectX::XMFLOAT4& SceneNode::GetCurrentAmbient()
{
    return mCameraAmbientInfo.mAmbientColor;
}

RSCamera* SceneNode::GetMainCamera()
{
    return mCameraAmbientInfo.mRSCameraPtr;
}

ActorObject* SceneNode::GetActorObject(std::string&& _actorName)
{
    return mObjContainerPtr->GetActorObject(_actorName);
}

ActorObject* SceneNode::GetActorObject(std::string& _actorName)
{
    return mObjContainerPtr->GetActorObject(_actorName);
}

void SceneNode::AddActorObject(class ActorObject& _newActor)
{
    mObjContainerPtr->AddActorObject(_newActor);
}

void SceneNode::DeleteActorObject(std::string&& _actorName)
{
    mObjContainerPtr->DeleteActorObject(_actorName);
}

void SceneNode::DeleteActorObject(std::string& _actorName)
{
    mObjContainerPtr->DeleteActorObject(_actorName);
}

UiObject* SceneNode::GetUiObject(std::string&& _uiName)
{
    return mObjContainerPtr->GetUiObject(_uiName);
}

UiObject* SceneNode::GetUiObject(std::string& _uiName)
{
    return mObjContainerPtr->GetUiObject(_uiName);
}

void SceneNode::AddUiObject(class UiObject& _newUi)
{
    mObjContainerPtr->AddUiObject(_newUi);
}

void SceneNode::DeleteUiObject(std::string&& _uiName)
{
    mObjContainerPtr->DeleteUiObject(_uiName);
}

void SceneNode::DeleteUiObject(std::string& _uiName)
{
    mObjContainerPtr->DeleteUiObject(_uiName);
}

AssetsPool* SceneNode::GetAssetsPool() const
{
    return mAssetsPoolPtr;
}

PhysicsWorld* SceneNode::GetPhysicsWorld() const
{
    return mPhysicsWorldPtr;
}

ComponentContainer* SceneNode::GetComponentContainer() const
{
    return mCompContainerPtr;
}

ObjectContainer* SceneNode::GetObjectContainer() const
{
    return mObjContainerPtr;
}
