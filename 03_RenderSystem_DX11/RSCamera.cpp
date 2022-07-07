//---------------------------------------------------------------
// File: RSCamera.cpp
// Proj: RenderSystem_DX11
// Info: 对一个摄像头的基本内容进行描述及相关处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSCamera.h"

using namespace dx;

RSCamera::RSCamera(const CAM_INFO *CamInfo)
    : LensType(LENS_TYPE::ORTHOGRAPHIC), CameraPosition({0.f, 0.f, 0.f}),
      CameraUpVector({0.f, 0.f, 0.f}), CameraLookAtVector({0.f, 0.f, 0.f}),
      PerspFovAngleY(0.f), AspectRatio(0.f), OrthoWidth(0.f), OrthoHeight(0.f),
      NearZ(0.f), FarZ(0.f), RSCameraInfo({}) {
  LensType = CamInfo->Type;
  CameraPosition = CamInfo->Position;
  CameraUpVector = CamInfo->UpVector;
  CameraLookAtVector = CamInfo->LookAtVector;
  PerspFovAngleY = CamInfo->PerspFovYRatio.x;
  OrthoWidth = CamInfo->OrthoWidthHeight.x;
  OrthoHeight = CamInfo->OrthoWidthHeight.y;
  NearZ = CamInfo->NearFarZ.x;
  FarZ = CamInfo->NearFarZ.y;
  switch (LensType) {
  case LENS_TYPE::PERSPECTIVE:
    AspectRatio = CamInfo->PerspFovYRatio.y;
    break;
  case LENS_TYPE::ORTHOGRAPHIC:
    AspectRatio = OrthoWidth / OrthoHeight;
    break;
  default:
    break;
  }

  calcViewMatrix();
  calcProjMatrix();
}

RSCamera::~RSCamera() {}

RS_CAM_INFO *RSCamera::getRSCameraInfo() { return &RSCameraInfo; }

void RSCamera::resetRSCamera(const CAM_INFO *CamInfo) {
  LensType = CamInfo->Type;
  CameraPosition = CamInfo->Position;
  CameraUpVector = CamInfo->UpVector;
  CameraLookAtVector = CamInfo->LookAtVector;
  PerspFovAngleY = CamInfo->PerspFovYRatio.x;
  OrthoWidth = CamInfo->OrthoWidthHeight.x;
  OrthoHeight = CamInfo->OrthoWidthHeight.y;
  NearZ = CamInfo->NearFarZ.x;
  FarZ = CamInfo->NearFarZ.y;
  switch (LensType) {
  case LENS_TYPE::PERSPECTIVE:
    AspectRatio = CamInfo->PerspFovYRatio.y;
    break;
  case LENS_TYPE::ORTHOGRAPHIC:
    AspectRatio = OrthoWidth / OrthoHeight;
    break;
  default:
    break;
  }

  calcViewMatrix();
  calcProjMatrix();
}

void RSCamera::translateRSCamera(const XMFLOAT3 &Delta) {
  XMVECTOR LookAtVec = XMLoadFloat3(&CameraLookAtVector);
  XMVECTOR UpVec = XMLoadFloat3(&CameraUpVector);
  XMVECTOR RightVec = XMVector3Cross(LookAtVec, UpVec);

  XMVECTOR MoveLookAtVec = XMVector3Normalize(LookAtVec);
  XMVECTOR MoveRightVec = XMVector3Normalize(RightVec);
  MoveLookAtVec *= Delta.z;
  MoveRightVec *= Delta.x;

  XMVECTOR Pos = XMLoadFloat3(&CameraPosition);
  Pos += MoveLookAtVec;
  Pos += MoveRightVec;

  XMStoreFloat3(&CameraPosition, Pos);

  calcViewMatrix();
}

void RSCamera::rotateRSCamera(float Vertical, float Horizontal) {
  XMVECTOR LookAtVec = XMLoadFloat3(&CameraLookAtVector);
  XMVECTOR UpVec = XMLoadFloat3(&CameraUpVector);
  XMVECTOR RightVec = XMVector3Cross(LookAtVec, UpVec);

  XMMATRIX PitchVec = XMMatrixRotationAxis(RightVec, Vertical);
  UpVec = XMVector3TransformNormal(UpVec, PitchVec);
  LookAtVec = XMVector3TransformNormal(LookAtVec, PitchVec);

  XMMATRIX YMatrix = XMMatrixRotationY(Horizontal);
  UpVec = XMVector3TransformNormal(UpVec, YMatrix);
  LookAtVec = XMVector3TransformNormal(LookAtVec, YMatrix);

  XMStoreFloat3(&CameraLookAtVector, LookAtVec);
  XMStoreFloat3(&CameraUpVector, UpVec);

  calcViewMatrix();
}

void RSCamera::rotateRSCamera(const XMFLOAT3 &DeltaAngle) {
  XMVECTOR LookAtVec = XMLoadFloat3(&CameraLookAtVector);
  XMVECTOR UpVec = XMLoadFloat3(&CameraUpVector);
  XMMATRIX XMatrix = XMMatrixRotationX(DeltaAngle.x);

  UpVec = XMVector3TransformNormal(UpVec, XMatrix);
  LookAtVec = XMVector3TransformNormal(LookAtVec, XMatrix);

  XMMATRIX YMatrix = XMMatrixRotationY(DeltaAngle.y);
  UpVec = XMVector3TransformNormal(UpVec, YMatrix);
  LookAtVec = XMVector3TransformNormal(LookAtVec, YMatrix);

  XMMATRIX ZMatrix = XMMatrixRotationZ(DeltaAngle.z);
  UpVec = XMVector3TransformNormal(UpVec, ZMatrix);
  LookAtVec = XMVector3TransformNormal(LookAtVec, ZMatrix);

  XMStoreFloat3(&CameraLookAtVector, LookAtVec);
  XMStoreFloat3(&CameraUpVector, UpVec);

  calcViewMatrix();
}

void RSCamera::changeRSCameraFovY(float FovYOrHeight) {
  switch (LensType) {
  case LENS_TYPE::PERSPECTIVE:
    PerspFovAngleY = FovYOrHeight;
    break;
  case LENS_TYPE::ORTHOGRAPHIC:
    OrthoHeight = FovYOrHeight;
    OrthoWidth = AspectRatio * OrthoHeight;
    break;
  default:
    return;
  }

  calcProjMatrix();
}

void RSCamera::changeRSCameraNearFarZ(float Near, float Far) {
  NearZ = Near;
  FarZ = Far;

  calcProjMatrix();
}

void RSCamera::changeRSCameraPosition(const DirectX::XMFLOAT3 &Position) {
  CameraPosition = Position;

  calcViewMatrix();
}

void RSCamera::resetRSCameraRotation(const DirectX::XMFLOAT3 &LookAtVector,
                                     const DirectX::XMFLOAT3 &UpVector) {
  CameraLookAtVector = LookAtVector;
  CameraUpVector = UpVector;

  calcViewMatrix();
}

void RSCamera::calcViewMatrix() {
  XMMATRIX ViewMat = XMMatrixLookAtLH(XMLoadFloat3(&CameraPosition),
                                      XMLoadFloat3(&CameraLookAtVector) +
                                          XMLoadFloat3(&CameraPosition),
                                      XMLoadFloat3(&CameraUpVector));
  XMVECTOR DetVec = XMMatrixDeterminant(ViewMat);
  XMMATRIX InvViewMat = XMMatrixInverse(&DetVec, ViewMat);
  XMMATRIX ViewProjMat =
      XMMatrixMultiply(ViewMat, XMLoadFloat4x4(&RSCameraInfo.ProjMatrix));

  XMStoreFloat4x4(&RSCameraInfo.ViewMatrix, ViewMat);
  XMStoreFloat4x4(&RSCameraInfo.InvViewMatrix, InvViewMat);
  XMStoreFloat4x4(&RSCameraInfo.ViewProjMatrix, ViewProjMat);

  RSCameraInfo.EyePosition = CameraPosition;
}

void RSCamera::calcProjMatrix() {
  XMMATRIX ProjMat = {};
  XMVECTOR DetVec = {};
  XMMATRIX InvProjMat = {};
  XMMATRIX ViewProjMat = {};
  switch (LensType) {
  case LENS_TYPE::PERSPECTIVE:
    ProjMat =
        XMMatrixPerspectiveFovLH(PerspFovAngleY, AspectRatio, NearZ, FarZ);
    DetVec = XMMatrixDeterminant(ProjMat);
    InvProjMat = XMMatrixInverse(&DetVec, ProjMat);
    ViewProjMat =
        XMMatrixMultiply(XMLoadFloat4x4(&RSCameraInfo.ViewMatrix), ProjMat);

    XMStoreFloat4x4(&RSCameraInfo.ProjMatrix, ProjMat);
    XMStoreFloat4x4(&RSCameraInfo.InvProjMatrix, InvProjMat);
    XMStoreFloat4x4(&RSCameraInfo.ViewProjMatrix, ViewProjMat);
    return;
  case LENS_TYPE::ORTHOGRAPHIC:
    ProjMat = XMMatrixOrthographicLH(OrthoWidth, OrthoHeight, NearZ, FarZ);
    DetVec = XMMatrixDeterminant(ProjMat);
    InvProjMat = XMMatrixInverse(&DetVec, ProjMat);
    ViewProjMat =
        XMMatrixMultiply(XMLoadFloat4x4(&RSCameraInfo.ViewMatrix), ProjMat);

    XMStoreFloat4x4(&RSCameraInfo.ProjMatrix, ProjMat);
    XMStoreFloat4x4(&RSCameraInfo.InvProjMatrix, InvProjMat);
    XMStoreFloat4x4(&RSCameraInfo.ViewProjMatrix, ViewProjMat);
    return;
  default:
    return;
  }
}
