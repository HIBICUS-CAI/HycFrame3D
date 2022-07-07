//---------------------------------------------------------------
// File: RSLight.h
// Proj: RenderSystem_DX11
// Info: 对一个光源的基本内容进行描述及相关处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSLight.h"

#include "RSCamera.h"
#include "RSCamerasContainer.h"
#include "RSDrawCallsPool.h"
#include "RSMeshHelper.h"
#include "RSResourceManager.h"
#include "RSRoot_DX11.h"

RSLight::RSLight(const LIGHT_INFO *Info)
    : LightType(Info->Type), EnabledShadowFlag(Info->ShadowFlag),
      LightIntensity(Info->Intensity), LightAlbedo(Info->Albedo),
      LightDirection(Info->Direction), LightPosition(Info->Position),
      LightFallOffStart(Info->FalloffStart), LightFallOffEnd(Info->FalloffEnd),
      LightSpotPower(Info->SpotPower),
      RSLightInfo({LightIntensity, LightAlbedo, LightFallOffStart,
                   LightDirection, LightFallOffEnd, LightPosition,
                   LightSpotPower}),
      RSLightCamera(nullptr), BloomLightFlag(false), BloomMeshData({}),
      BloomInstanceData({}), BloomDrawCallData({}) {}

RSLight::~RSLight() {}

RS_LIGHT_INFO *RSLight::getRSLightInfo() { return &RSLightInfo; }

LIGHT_TYPE
RSLight::getRSLightType() { return LightType; }

void RSLight::resetRSLight(const LIGHT_INFO *Info) {
  LightType = Info->Type;
  setRSLightIntensity(Info->Intensity);
  setRSLightAlbedo(Info->Albedo);
  setRSLightDirection(Info->Direction);
  setRSLightPosition(Info->Position);
  setRSLightFallOff(Info->FalloffStart, Info->FalloffEnd);
  setRSLightSpotPower(Info->SpotPower);
}

void RSLight::setRSLightAlbedo(const dx::XMFLOAT3 &Albedo) {
  LightAlbedo = Albedo;
  RSLightInfo.Albedo = Albedo;
}

void RSLight::setRSLightDirection(const dx::XMFLOAT3 &Direction) {
  LightDirection = Direction;
  RSLightInfo.Direction = Direction;
}

void RSLight::setRSLightPosition(const dx::XMFLOAT3 &Position) {
  LightPosition = Position;
  RSLightInfo.Position = Position;

  if (RSLightCamera) {
    RSLightCamera->changeRSCameraPosition(Position);
  }

  if (BloomLightFlag) {
    dx::XMMATRIX WorldMat = dx::XMMatrixTranslation(
        LightPosition.x, LightPosition.y, LightPosition.z);
    dx::XMStoreFloat4x4(&(BloomInstanceData[0].WorldMatrix), WorldMat);
  }
}

void RSLight::setRSLightFallOff(float Start, float End) {
  LightFallOffStart = Start;
  LightFallOffEnd = End;
  RSLightInfo.FalloffStart = Start;
  RSLightInfo.FalloffEnd = End;
}

void RSLight::setRSLightSpotPower(float SpotPower) {
  LightSpotPower = SpotPower;
  RSLightInfo.SpotPower = SpotPower;
}

void RSLight::setRSLightIntensity(float Luminance) {
  LightIntensity = Luminance;
  RSLightInfo.Intensity = Luminance;
}

RSCamera *RSLight::createLightCamera(const std::string &LightName,
                                     const CAM_INFO *Info,
                                     RSCamerasContainer *CameraContainer) {
  if (!Info || !CameraContainer) {
    return nullptr;
  }

  std::string CameraName = LightName + "-light-cam";
  RSLightCamera = CameraContainer->createRSCamera(CameraName, Info);

  return RSLightCamera;
}

RSCamera *RSLight::getRSLightCamera() { return RSLightCamera; }

void RSLight::setLightBloom(const RS_SUBMESH_DATA &MeshData) {
  BloomLightFlag = true;
  BloomMeshData = MeshData;
  BloomInstanceData.resize(1);

  BloomInstanceData[0].CustomizedData1.x = LightAlbedo.x;
  BloomInstanceData[0].CustomizedData1.y = LightAlbedo.y;
  BloomInstanceData[0].CustomizedData1.z = LightAlbedo.z;
  BloomInstanceData[0].CustomizedData1.w = LightIntensity;
  dx::XMMATRIX WorldMat = dx::XMMatrixTranslation(
      LightPosition.x, LightPosition.y, LightPosition.z);
  dx::XMStoreFloat4x4(&(BloomInstanceData[0].WorldMatrix), WorldMat);

  BloomDrawCallData.InstanceData.DataArrayPtr = &BloomInstanceData;
  BloomDrawCallData.MeshData.IndexBuffer = BloomMeshData.IndexBuffer;
  BloomDrawCallData.MeshData.VertexBuffer = BloomMeshData.VertexBuffer;
  BloomDrawCallData.MeshData.IndexSize = BloomMeshData.IndexSize;
  BloomDrawCallData.MeshData.InputLayout = BloomMeshData.InputLayout;
  BloomDrawCallData.MeshData.TopologyType = BloomMeshData.TopologyType;
}

void RSLight::updateBloomColor() {
  if (BloomLightFlag) {
    BloomInstanceData[0].CustomizedData1.x = LightAlbedo.x;
    BloomInstanceData[0].CustomizedData1.y = LightAlbedo.y;
    BloomInstanceData[0].CustomizedData1.z = LightAlbedo.z;
    BloomInstanceData[0].CustomizedData1.w = LightIntensity;
  }
}

void RSLight::uploadLightDrawCall() {
  static auto DrawCallPool = getRSDX11RootInstance()->getDrawCallsPool();
  if (BloomLightFlag) {
    DrawCallPool->addDrawCallToPipe(DRAWCALL_TYPE::LIGHT, BloomDrawCallData);
  }
}

void RSLight::releaseLightBloom(bool DeleteByFrameworkFlag) {
  if (BloomLightFlag && !DeleteByFrameworkFlag) {
    getRSDX11RootInstance()->getMeshHelper()->releaseSubMesh(BloomMeshData);
  }
}

dx::XMFLOAT4X4 *RSLight::getLightWorldMat() {
  if (!BloomInstanceData.size()) {
    return nullptr;
  }
  return &(BloomInstanceData[0].WorldMatrix);
}
