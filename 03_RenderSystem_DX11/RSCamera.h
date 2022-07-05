//---------------------------------------------------------------
// File: RSCamera.h
// Proj: RenderSystem_DX11
// Info: 对一个摄像头的基本内容进行描述及相关处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

class RSCamera {
private:
  LENS_TYPE LensType;

  DirectX::XMFLOAT3 CameraPosition;
  DirectX::XMFLOAT3 CameraUpVector;     // 基于原点的向量
  DirectX::XMFLOAT3 CameraLookAtVector; // 基于原点的向量

  float PerspFovAngleY;
  float AspectRatio;
  float OrthoWidth;
  float OrthoHeight;
  float NearZ;
  float FarZ;

  RS_CAM_INFO RSCameraInfo;

public:
  RSCamera(const CAM_INFO *CamInfo);
  ~RSCamera();

  RS_CAM_INFO *
  getRSCameraInfo();

  void
  resetRSCamera(const CAM_INFO *CamInfo);

  void
  translateRSCamera(const DirectX::XMFLOAT3& Delta);

  void
  rotateRSCamera(float Vertical, float Horizontal);

  void
  rotateRSCamera(const DirectX::XMFLOAT3& DeltaAngle);

  void
  changeRSCameraFovY(float Angle);

  void
  changeRSCameraNearFarZ(float Near, float Far);

  const DirectX::XMFLOAT3&
  getRSCameraPosition() {
    return CameraPosition;
  }

  const DirectX::XMFLOAT3&
  getRSCameraUpVector() {
    return CameraUpVector;
  }

  const DirectX::XMFLOAT3&
  getRSCameraLookDir() {
    return CameraLookAtVector;
  }

  void
  changeRSCameraPosition(const DirectX::XMFLOAT3 &Position);

  void
  resetRSCameraRotation(const DirectX::XMFLOAT3 &LookAtVector,
                        const DirectX::XMFLOAT3 &UpVector);

private:
  void
  calcViewMatrix();

  void
  calcProjMatrix();
};
