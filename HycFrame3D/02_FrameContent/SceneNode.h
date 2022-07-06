#pragma once

#include "Hyc3DCommon.h"

#include <directxmath.h>
#include <string>

namespace dx = DirectX;

struct CAMERA_AND_AMBIENT {
  class RSCamera *RSCameraPtr = nullptr;
  dx::XMFLOAT4 AmbientFactor = {};
  struct ID3D11ShaderResourceView *IBLEnvironmentTexture = nullptr;
  struct ID3D11ShaderResourceView *IBLDiffuseTexture = nullptr;
  struct ID3D11ShaderResourceView *IBLSpecularTexture = nullptr;
};

class SceneNode {
private:
  const std::string SceneName;
  class SceneManager *SceneManagerPtr;
  class ObjectContainer *ObjectContainerPtr;
  class ComponentContainer *ComponentContainerPtr;
  class AssetsPool *AssetsPoolPtr;
  class PhysicsWorld *PhysicsWorldPtr;
  CAMERA_AND_AMBIENT CameraAmbientInfo;

public:
  SceneNode(const std::string &SceneName, class SceneManager *SceneManager);
  ~SceneNode();

  void
  releaseScene();

  const std::string &
  getSceneNodeName() const;
  class SceneManager *
  getSceneManager() const;

  void
  setCurrentAmbientFactor(const dx::XMFLOAT4 &_ambientColor);
  const dx::XMFLOAT4 &
  getCurrentAmbientFactor();

  void
  loadIBLTexture(const std::string &Env,
                 const std::string &Diff,
                 const std::string &Spec);
  struct ID3D11ShaderResourceView *
  getIBLEnvironment();
  struct ID3D11ShaderResourceView *
  getIBLDiffuse();
  struct ID3D11ShaderResourceView *
  getIBLSpecular();

  class RSCamera *
  getMainCamera();

  class ActorObject *
  getActorObject(const std::string &ActorName);
  void
  addActorObject(const class ActorObject &NewActor);
  void
  deleteActorObject(const std::string &ActorName);
  class UiObject *
  getUiObject(const std::string &UiName);
  void
  addUiObject(const class UiObject &NewUi);
  void
  deleteUiObject(const std::string &UiName);

  class AssetsPool *
  getAssetsPool() const;
  class PhysicsWorld *
  getPhysicsWorld() const;
  class ObjectContainer *
  getObjectContainer() const;
  class ComponentContainer *
  getComponentContainer() const;
};
