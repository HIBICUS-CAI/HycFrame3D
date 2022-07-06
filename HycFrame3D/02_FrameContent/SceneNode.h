#pragma once

#include "Hyc3DCommon.h"
#include <string>
#include <DirectXMath.h>

struct CAMERA_AND_AMBIENT
{
    class RSCamera* mRSCameraPtr = nullptr;
    DirectX::XMFLOAT4 mAmbientFactor = {};
    struct ID3D11ShaderResourceView* mIBLEnvTex = nullptr;
    struct ID3D11ShaderResourceView* mIBLDiffTex = nullptr;
    struct ID3D11ShaderResourceView* mIBLSpecTex = nullptr;
};

class SceneNode
{
public:
    SceneNode(const std::string& _sceneName, class SceneManager* _sceneManager);
    ~SceneNode();

    void ReleaseScene();

    const std::string& GetSceneNodeName() const;
    class SceneManager* GetSceneManager() const;

    void SetCurrentAmbientFactor(DirectX::XMFLOAT4&& _ambientColor);
    void SetCurrentAmbientFactor(DirectX::XMFLOAT4& _ambientColor);
    DirectX::XMFLOAT4& GetCurrentAmbientFactor();

    void LoadIBLTexture(std::string _env = "", std::string _diff = "",
        std::string _spc = "");
    struct ID3D11ShaderResourceView* GetIBLEnvironment();
    struct ID3D11ShaderResourceView* GetIBLDiffuse();
    struct ID3D11ShaderResourceView* GetIBLSpecular();

    class RSCamera* GetMainCamera();

    class ActorObject* GetActorObject(const std::string& _actorName);
    void AddActorObject(class ActorObject& _newActor);
    void DeleteActorObject(std::string&& _actorName);
    void DeleteActorObject(std::string& _actorName);
    class UiObject* GetUiObject(const std::string& _uiName);
    void AddUiObject(class UiObject& _newUi);
    void DeleteUiObject(std::string&& _uiName);
    void DeleteUiObject(std::string& _uiName);

    class AssetsPool* GetAssetsPool() const;
    class PhysicsWorld* GetPhysicsWorld() const;
    class ObjectContainer* GetObjectContainer() const;
    class ComponentContainer* GetComponentContainer() const;

private:
    const std::string mSceneName;
    class SceneManager* mSceneManagerPtr;
    class ObjectContainer* mObjContainerPtr;
    class ComponentContainer* mCompContainerPtr;
    class AssetsPool* mAssetsPoolPtr;
    class PhysicsWorld* mPhysicsWorldPtr;
    CAMERA_AND_AMBIENT mCameraAmbientInfo;
};
