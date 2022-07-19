#include "StaticObstacle.h"

#include "BulletProcess.h"
#include "PlayerProcess.h"

using namespace dx;

void registerStaticObstacle(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInitMapPtr().insert({FUNC_NAME(obstacleInit), obstacleInit});
  Factory->getAUpdateMapPtr().insert(
      {FUNC_NAME(obstacleUpdate), obstacleUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(obstacleDestory), obstacleDestory});
}

bool obstacleInit(AInteractComponent *Aitc) { return true; }

void obstacleUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  auto Acc = Aitc->getActorOwner()->getComponent<ACollisionComponent>();
  static CONTACT_PONT_PAIR Contact = {};
  if (Acc->checkCollisionWith(PLAYER_NAME, &Contact)) {
    dx::XMFLOAT3 ContactPnt = ACollisionComponent::calcCenterOfContact(Contact);
    auto PlayerAtc =
        Aitc->getActorObject(PLAYER_NAME)->getComponent<ATransformComponent>();
    float XOffset = fabsf(ContactPnt.x - PlayerAtc->getProcessingPosition().x);
    float ZOffset = fabsf(ContactPnt.z - PlayerAtc->getProcessingPosition().z);
    if (getPlayerIsDashingFlg()) {
      PlayerAtc->rollBackPosition();
      setPlayerDashToObstacle();
    } else if (XOffset < 3.1f && ZOffset < 3.1f &&
               (ContactPnt.y - PlayerAtc->getProcessingPosition().y) < -0.1f) {
      XMFLOAT3 PlayerPnt = PlayerAtc->getProcessingPosition();
      XMVECTOR ContactVec =
          XMLoadFloat3(&Contact.first) - XMLoadFloat3(&PlayerPnt);
      float LengthFirSq = XMVectorGetX(XMVector3LengthSq(ContactVec));
      ContactVec = XMLoadFloat3(&Contact.second) - XMLoadFloat3(&PlayerPnt);
      float LengthSecSq = XMVectorGetX(XMVector3LengthSq(ContactVec));
      float XZSq = XOffset * XOffset + ZOffset * ZOffset;
      float YFir = sqrtf(fabsf(LengthFirSq - XZSq));
      float YSec = sqrtf(fabsf(LengthSecSq - XZSq));
      float Offset = fabsf(YFir - YSec);
      PlayerAtc->rollBackPositionY();
      PlayerAtc->translateYAsix(Offset);
      setPlayerContactGround();
    } else if (XOffset < 3.1f && ZOffset < 3.1f &&
               (ContactPnt.y - PlayerAtc->getProcessingPosition().y) > 2.5f) {
      PlayerAtc->rollBackPositionY();

      XMFLOAT3 PlayerPnt = PlayerAtc->getProcessingPosition();
      XMVECTOR ContactVec =
          XMLoadFloat3(&Contact.first) - XMLoadFloat3(&PlayerPnt);
      float LengthFirSq = XMVectorGetX(XMVector3LengthSq(ContactVec));
      ContactVec = XMLoadFloat3(&Contact.second) - XMLoadFloat3(&PlayerPnt);
      float LengthSecSq = XMVectorGetX(XMVector3LengthSq(ContactVec));
      float XZSq = XOffset * XOffset + ZOffset * ZOffset;
      float YFir = sqrtf(LengthFirSq - XZSq);
      float YSec = sqrtf(LengthSecSq - XZSq);
      float Offset = fabsf(YFir - YSec);
      PlayerAtc->translateYAsix(-Offset);

      setPlayerBrokeHead();
    } else {
      static const dx::XMFLOAT3 X_UNIT = {1.f, 0.f, 0.f};
      static const dx::XMFLOAT3 Z_UNIT = {0.f, 0.f, 1.f};
      static const dx::XMVECTOR X_UVEC = dx::XMLoadFloat3(&X_UNIT);
      static const dx::XMVECTOR Z_UVEC = dx::XMLoadFloat3(&Z_UNIT);
      dx::XMFLOAT3 PlayerMove = getPlayerMoveDirection();
      dx::XMFLOAT3 PlayerPnt = PlayerAtc->getProcessingPosition();
      dx::XMVECTOR PlyMoveVec = dx::XMLoadFloat3(&PlayerMove);
      dx::XMVECTOR ContactVec =
          dx::XMLoadFloat3(&ContactPnt) - dx::XMLoadFloat3(&PlayerPnt);
      ContactVec = dx::XMVector3Normalize(ContactVec);
      ContactVec = dx::XMVectorGetX(dx::XMVector3Dot(ContactVec, PlyMoveVec)) *
                   ContactVec;
      dx::XMVECTOR MoveVec = PlyMoveVec - ContactVec;
      float XAsix = dx::XMVectorGetX(dx::XMVector3Dot(MoveVec, X_UVEC));
      float ZAsix = dx::XMVectorGetX(dx::XMVector3Dot(MoveVec, Z_UVEC));
      float AimSlow =
          (getPlayerAimingFlg() && getPlayerCanAimFlg()) ? 0.1f : 1.f;
      float Deltatime = Timer.floatDeltaTime() * AimSlow;
      PlayerAtc->rollBackPositionX();
      PlayerAtc->rollBackPositionZ();
      PlayerAtc->translateZAsix(0.02f * Deltatime * ZAsix);
      PlayerAtc->translateXAsix(0.02f * Deltatime * XAsix);
      dx::XMStoreFloat3(&PlayerMove, MoveVec);
      setPlayerMoveDirection(PlayerMove);
    }
  }
}

void obstacleDestory(AInteractComponent *Aitc) {}
