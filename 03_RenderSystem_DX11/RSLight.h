//---------------------------------------------------------------
// File: RSLight.h
// Proj: RenderSystem_DX11
// Info: 对一个光源的基本内容进行描述及相关处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

class RSLight {
private:
  LIGHT_TYPE LightType;

  bool EnabledShadowFlag;

  float LightIntensity;
  dx::XMFLOAT3 LightAlbedo;
  dx::XMFLOAT3 LightDirection;
  dx::XMFLOAT3 LightPosition;
  float LightFallOffStart;
  float LightFallOffEnd;
  float LightSpotPower;

  RS_LIGHT_INFO RSLightInfo;
  class RSCamera *RSLightCamera;

  bool BloomLightFlag;
  RS_SUBMESH_DATA BloomMeshData;
  std::vector<RS_INSTANCE_DATA> BloomInstanceData;
  RS_DRAWCALL_DATA BloomDrawCallData;

public:
  RSLight(const LIGHT_INFO *Info);
  ~RSLight();

  class RSCamera *createLightCamera(const std::string &LightName,
                                    const CAM_INFO *Info,
                                    class RSCamerasContainer *CameraContainer);

  RS_LIGHT_INFO *getRSLightInfo();
  LIGHT_TYPE getRSLightType();
  class RSCamera *getRSLightCamera();

  void resetRSLight(const LIGHT_INFO *Info);

  DirectX::XMFLOAT4X4 *getLightWorldMat();

  void setRSLightAlbedo(const DirectX::XMFLOAT3 &Albedo);
  void setRSLightDirection(const DirectX::XMFLOAT3 &Direction);
  void setRSLightPosition(const DirectX::XMFLOAT3 &Position);
  void setRSLightFallOff(float Start, float End);
  void setRSLightSpotPower(float SpotPower);
  void setRSLightIntensity(float Luminance);
  void setLightBloom(const RS_SUBMESH_DATA &MeshData);

  void uploadLightDrawCall();
  void releaseLightBloom(bool DeleteByFrameworkFlag);
  void updateBloomColor();
};
