#include "SceneNode.h"
#include <vector>
#include "DDSTextureLoader11.h"
#include "RSDevices.h"
#include "RSCamerasContainer.h"
#include "RSCamera.h"
#include "RSRoot_DX11.h"
#include "ObjectContainer.h"
#include "ComponentContainer.h"
#include "AssetsPool.h"
#include "PhysicsWorld.h"

SceneNode::SceneNode(const std::string& _sceneName, SceneManager* _sceneManager) :
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
        mCameraAmbientInfo.mRSCameraPtr = getRSDX11RootInstance()->
            getCamerasContainer()->getRSCamera(cam);
        bool new_scene_fail = mObjContainerPtr && mCompContainerPtr &&
            mAssetsPoolPtr && mPhysicsWorldPtr && mCameraAmbientInfo.mRSCameraPtr;
        assert(new_scene_fail);
        (void)new_scene_fail;
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
    mObjContainerPtr->deleteAllActor();
    mObjContainerPtr->deleteAllUi();
    mObjContainerPtr->deleteAllDeadObjects();
    mCompContainerPtr->deleteAllComponent();
    mPhysicsWorldPtr->DeletePhysicsWorld();
    mAssetsPoolPtr->deleteAllAssets();

    if (mCameraAmbientInfo.mIBLEnvTex)
    {
        mCameraAmbientInfo.mIBLEnvTex->Release();
        mCameraAmbientInfo.mIBLEnvTex = nullptr;
    }
    if (mCameraAmbientInfo.mIBLDiffTex)
    {
        mCameraAmbientInfo.mIBLDiffTex->Release();
        mCameraAmbientInfo.mIBLDiffTex = nullptr;
    }
    if (mCameraAmbientInfo.mIBLSpecTex)
    {
        mCameraAmbientInfo.mIBLSpecTex->Release();
        mCameraAmbientInfo.mIBLSpecTex = nullptr;
    }
}

const std::string& SceneNode::GetSceneNodeName() const
{
    return mSceneName;
}

SceneManager* SceneNode::GetSceneManager() const
{
    return mSceneManagerPtr;
}

void SceneNode::SetCurrentAmbientFactor(DirectX::XMFLOAT4&& _ambientColor)
{
    mCameraAmbientInfo.mAmbientFactor = _ambientColor;
}

void SceneNode::SetCurrentAmbientFactor(DirectX::XMFLOAT4& _ambientColor)
{
    mCameraAmbientInfo.mAmbientFactor = _ambientColor;
}

DirectX::XMFLOAT4& SceneNode::GetCurrentAmbientFactor()
{
    return mCameraAmbientInfo.mAmbientFactor;
}

void SceneNode::LoadIBLTexture(std::string _env, std::string _diff, std::string _spc)
{
    std::wstring wstr = L"";
    HRESULT hr = S_OK;
    ID3D11ShaderResourceView* srv = nullptr;

    if (_env != "")
    {
        wstr = std::wstring(_env.begin(), _env.end());
        wstr = L".\\Assets\\Textures\\" + wstr;
        hr = DirectX::CreateDDSTextureFromFile(
            getRSDX11RootInstance()->getDevices()->getDevice(),
            wstr.c_str(), nullptr, &srv);
        assert(!FAILED(hr));
        (void)hr;
        mCameraAmbientInfo.mIBLEnvTex = srv;
    }

    if (_diff != "")
    {
        wstr = std::wstring(_diff.begin(), _diff.end());
        wstr = L".\\Assets\\Textures\\" + wstr;
        hr = DirectX::CreateDDSTextureFromFile(
            getRSDX11RootInstance()->getDevices()->getDevice(),
            wstr.c_str(), nullptr, &srv);
        assert(!FAILED(hr));
        (void)hr;
        mCameraAmbientInfo.mIBLDiffTex = srv;
    }

    if (_spc != "")
    {
        wstr = std::wstring(_spc.begin(), _spc.end());
        wstr = L".\\Assets\\Textures\\" + wstr;
        hr = DirectX::CreateDDSTextureFromFile(
            getRSDX11RootInstance()->getDevices()->getDevice(),
            wstr.c_str(), nullptr, &srv);
        assert(!FAILED(hr));
        (void)hr;
        mCameraAmbientInfo.mIBLSpecTex = srv;
    }    
}

ID3D11ShaderResourceView* SceneNode::GetIBLEnvironment()
{
    return mCameraAmbientInfo.mIBLEnvTex;
}

ID3D11ShaderResourceView* SceneNode::GetIBLDiffuse()
{
    return mCameraAmbientInfo.mIBLDiffTex;
}

ID3D11ShaderResourceView* SceneNode::GetIBLSpecular()
{
    return mCameraAmbientInfo.mIBLSpecTex;
}

RSCamera* SceneNode::GetMainCamera()
{
    return mCameraAmbientInfo.mRSCameraPtr;
}

ActorObject* SceneNode::GetActorObject(const std::string& _actorName)
{
    return mObjContainerPtr->getActorObject(_actorName);
}

void SceneNode::AddActorObject(class ActorObject& _newActor)
{
    mObjContainerPtr->addActorObject(_newActor);
}

void SceneNode::DeleteActorObject(std::string&& _actorName)
{
    mObjContainerPtr->deleteActorObject(_actorName);
}

void SceneNode::DeleteActorObject(std::string& _actorName)
{
    mObjContainerPtr->deleteActorObject(_actorName);
}

UiObject* SceneNode::GetUiObject(const std::string& _uiName)
{
    return mObjContainerPtr->getUiObject(_uiName);
}

void SceneNode::AddUiObject(class UiObject& _newUi)
{
    mObjContainerPtr->addUiObject(_newUi);
}

void SceneNode::DeleteUiObject(std::string&& _uiName)
{
    mObjContainerPtr->deleteUiObject(_uiName);
}

void SceneNode::DeleteUiObject(std::string& _uiName)
{
    mObjContainerPtr->deleteUiObject(_uiName);
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
