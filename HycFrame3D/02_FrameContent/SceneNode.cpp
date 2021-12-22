#include "SceneNode.h"
#include <vector>
#include "RSCamerasContainer.h"
#include "RSCamera.h"
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
    assert(mObjContainerPtr && mCompContainerPtr && mAssetsPoolPtr && mPhysicsWorldPtr);
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
    mPhysicsWorldPtr->DeletePhysicsWorld();
    mAssetsPoolPtr->DeleteAllAssets();
    mCompContainerPtr->DeleteAllComponent();
    mObjContainerPtr->DeleteAllActor();
    mObjContainerPtr->DeleteAllUi();
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
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

ActorObject* SceneNode::GetActorObject(std::string& _actorName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

void SceneNode::AddActorObject(class ActorObject& _newActor)
{

}

void SceneNode::DeleteActorObject(std::string&& _actorName)
{

}

void SceneNode::DeleteActorObject(std::string& _actorName_actorName)
{

}

UiObject* SceneNode::GetUiObject(std::string&& _uiName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

UiObject* SceneNode::GetUiObject(std::string& _uiName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

void SceneNode::AddUiObject(class UiObject& _newUi)
{

}

void SceneNode::DeleteUiObject(std::string&& _uiName)
{

}

void SceneNode::DeleteUiObject(std::string& _uiName)
{

}

AssetsPool* SceneNode::GetAssetsPool() const
{
    return mAssetsPoolPtr;
}

PhysicsWorld* SceneNode::GetPhysicsWorld() const
{
    return mPhysicsWorldPtr;
}
