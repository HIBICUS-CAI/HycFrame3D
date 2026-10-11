#include "SceneNode.h"

#include "AssetsPool.h"
#include "ComponentContainer.h"
#include "ObjectContainer.h"
#include "PhysicsWorld.h"

#include <RSCamera.h>
#include <RSCamerasContainer.h>
#include <RSDevices.h>
#include <RSRoot_DX11.h>
#include <RSTextureLoader.h>
#include <RSVkObjects.h>

#include <vector>

SceneNode::SceneNode(const std::string &Name, SceneManager *SceneManager)
    : SceneName(Name), SceneManagerPtr(SceneManager),
      ObjectContainerPtr(nullptr), ComponentContainerPtr(nullptr),
      AssetsPoolPtr(nullptr), PhysicsWorldPtr(nullptr), CameraAmbientInfo({}) {
  ObjectContainerPtr = new ObjectContainer(*this);
  ComponentContainerPtr = new ComponentContainer(*this);
  AssetsPoolPtr = new AssetsPool(*this);
  PhysicsWorldPtr = new PhysicsWorld(*this);
  if (Name != "temp-loading-scene") {
    std::string Cam = "temp-cam";
    CameraAmbientInfo.RSCameraPtr =
        getRSDX11RootInstance()->getCamerasContainer()->getRSCamera(Cam);
    assert(ObjectContainerPtr && ComponentContainerPtr && AssetsPoolPtr &&
           PhysicsWorldPtr && CameraAmbientInfo.RSCameraPtr &&
           "new scene fail");
  }
}

SceneNode::~SceneNode() {
  delete PhysicsWorldPtr;
  delete AssetsPoolPtr;
  delete ComponentContainerPtr;
  delete ObjectContainerPtr;
}

void SceneNode::releaseScene() {
  ObjectContainerPtr->deleteAllActor();
  ObjectContainerPtr->deleteAllUi();
  ObjectContainerPtr->deleteAllDeadObjects();
  ComponentContainerPtr->deleteAllComponent();
  PhysicsWorldPtr->deletePhysicsWorld();
  AssetsPoolPtr->deleteAllAssets();

  if (CameraAmbientInfo.IBLEnvironmentTexture) {
    CameraAmbientInfo.IBLEnvironmentTexture->Release();
    CameraAmbientInfo.IBLEnvironmentTexture = nullptr;
  }
  if (CameraAmbientInfo.IBLDiffuseTexture) {
    CameraAmbientInfo.IBLDiffuseTexture->Release();
    CameraAmbientInfo.IBLDiffuseTexture = nullptr;
  }
  if (CameraAmbientInfo.IBLSpecularTexture) {
    CameraAmbientInfo.IBLSpecularTexture->Release();
    CameraAmbientInfo.IBLSpecularTexture = nullptr;
  }
}

const std::string &SceneNode::getSceneNodeName() const { return SceneName; }

SceneManager *SceneNode::getSceneManager() const { return SceneManagerPtr; }

void SceneNode::setCurrentAmbientFactor(const DirectX::XMFLOAT4 &Factor) {
  CameraAmbientInfo.AmbientFactor = Factor;
}

const DirectX::XMFLOAT4 &SceneNode::getCurrentAmbientFactor() {
  return CameraAmbientInfo.AmbientFactor;
}

void SceneNode::loadIBLTexture(const std::string &Env,
                               const std::string &Diff,
                               const std::string &Spec) {
  bool Result = false;
  RSShaderResourceView *Srv = nullptr;
  RSDevices *Devices = getRSDX11RootInstance()->getDevices();

  if (Env != "") {
    Result = rs_tool::createDDSTextureFromFile(
        Devices, "./Assets/Textures/" + Env, nullptr, &Srv);
    assert(Result && "fail to load IBL env texture");
    (void)Result;
    CameraAmbientInfo.IBLEnvironmentTexture = Srv;
  }

  if (Diff != "") {
    Result = rs_tool::createDDSTextureFromFile(
        Devices, "./Assets/Textures/" + Diff, nullptr, &Srv);
    assert(Result && "fail to load IBL diff texture");
    (void)Result;
    CameraAmbientInfo.IBLDiffuseTexture = Srv;
  }

  if (Spec != "") {
    Result = rs_tool::createDDSTextureFromFile(
        Devices, "./Assets/Textures/" + Spec, nullptr, &Srv);
    assert(Result && "fail to load IBL spec texture");
    (void)Result;
    CameraAmbientInfo.IBLSpecularTexture = Srv;
  }
}

RSShaderResourceView *SceneNode::getIBLEnvironment() {
  return CameraAmbientInfo.IBLEnvironmentTexture;
}

RSShaderResourceView *SceneNode::getIBLDiffuse() {
  return CameraAmbientInfo.IBLDiffuseTexture;
}

RSShaderResourceView *SceneNode::getIBLSpecular() {
  return CameraAmbientInfo.IBLSpecularTexture;
}

RSCamera *SceneNode::getMainCamera() { return CameraAmbientInfo.RSCameraPtr; }

ActorObject *SceneNode::getActorObject(const std::string &ActorName) {
  return ObjectContainerPtr->getActorObject(ActorName);
}

void SceneNode::addActorObject(const class ActorObject &NewActor) {
  ObjectContainerPtr->addActorObject(NewActor);
}

void SceneNode::deleteActorObject(const std::string &ActorName) {
  ObjectContainerPtr->deleteActorObject(ActorName);
}

UiObject *SceneNode::getUiObject(const std::string &UiName) {
  return ObjectContainerPtr->getUiObject(UiName);
}

void SceneNode::addUiObject(const class UiObject &NewUi) {
  ObjectContainerPtr->addUiObject(NewUi);
}

void SceneNode::deleteUiObject(const std::string &UiName) {
  ObjectContainerPtr->deleteUiObject(UiName);
}

AssetsPool *SceneNode::getAssetsPool() const { return AssetsPoolPtr; }

PhysicsWorld *SceneNode::getPhysicsWorld() const { return PhysicsWorldPtr; }

ComponentContainer *SceneNode::getComponentContainer() const {
  return ComponentContainerPtr;
}

ObjectContainer *SceneNode::getObjectContainer() const {
  return ObjectContainerPtr;
}
