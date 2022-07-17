#include "StaticObstacle.h"
#include "PlayerProcess.h"
#include "BulletProcess.h"

using namespace DirectX;

void RegisterStaticObstacle(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(ObstacleInit),ObstacleInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(ObstacleUpdate),ObstacleUpdate });
    _factory->getADestoryMapPtr().insert(
        { FUNC_NAME(ObstacleDestory),ObstacleDestory });
}

bool ObstacleInit(AInteractComponent* _aitc)
{
    return true;
}

void ObstacleUpdate(AInteractComponent* _aitc, const Timer& _timer)
{
    auto acc = _aitc->getActorOwner()->
        getComponent<ACollisionComponent>();
    static CONTACT_PONT_PAIR contact = {};
    if (acc->checkCollisionWith(PLAYER_NAME, &contact))
    {
        DirectX::XMFLOAT3 contactPnt =
            ACollisionComponent::calcCenterOfContact(contact);
        auto playerAtc = _aitc->getActorOwner()->getSceneNode().
            getActorObject(PLAYER_NAME)->
            getComponent<ATransformComponent>();
        float xOffset = fabsf(contactPnt.x -
            playerAtc->getProcessingPosition().x);
        float zOffset = fabsf(contactPnt.z -
            playerAtc->getProcessingPosition().z);
        if (GetPlayerIsDashingFlg())
        {
            playerAtc->rollBackPosition();
            SetPlayerDashToObstacle();
        }
        else if (xOffset < 3.1f && zOffset < 3.1f &&
            (contactPnt.y - playerAtc->getProcessingPosition().y) < -0.1f)
        {
            XMFLOAT3 playerPnt = playerAtc->getProcessingPosition();
            XMVECTOR contactVec = XMLoadFloat3(&contact.first) -
                XMLoadFloat3(&playerPnt);
            float lengthFirSq = XMVectorGetX(XMVector3LengthSq(contactVec));
            contactVec = XMLoadFloat3(&contact.second) -
                XMLoadFloat3(&playerPnt);
            float lengthSecSq = XMVectorGetX(XMVector3LengthSq(contactVec));
            float xzSq = xOffset * xOffset + zOffset * zOffset;
            float yFir = sqrtf(fabsf(lengthFirSq - xzSq));
            float ySec = sqrtf(fabsf(lengthSecSq - xzSq));
            float offset = fabsf(yFir - ySec);
            playerAtc->rollBackPositionY();
            playerAtc->translateYAsix(offset);
            SetPlayerContactGround();
        }
        else if (xOffset < 3.1f && zOffset < 3.1f &&
            (contactPnt.y - playerAtc->getProcessingPosition().y) > 2.5f)
        {
            playerAtc->rollBackPositionY();

            XMFLOAT3 playerPnt = playerAtc->getProcessingPosition();
            XMVECTOR contactVec = XMLoadFloat3(&contact.first) -
                XMLoadFloat3(&playerPnt);
            float lengthFirSq = XMVectorGetX(XMVector3LengthSq(contactVec));
            contactVec = XMLoadFloat3(&contact.second) -
                XMLoadFloat3(&playerPnt);
            float lengthSecSq = XMVectorGetX(XMVector3LengthSq(contactVec));
            float xzSq = xOffset * xOffset + zOffset * zOffset;
            float yFir = sqrtf(lengthFirSq - xzSq);
            float ySec = sqrtf(lengthSecSq - xzSq);
            float offset = fabsf(yFir - ySec);
            playerAtc->translateYAsix(-offset);

            SetPlayerBrokeHead();
        }
        else
        {
            static const DirectX::XMFLOAT3 xUnit = { 1.f,0.f,0.f };
            static const DirectX::XMFLOAT3 zUnit = { 0.f,0.f,1.f };
            static const DirectX::XMVECTOR xVec = DirectX::XMLoadFloat3(&xUnit);
            static const DirectX::XMVECTOR zVec = DirectX::XMLoadFloat3(&zUnit);
            DirectX::XMFLOAT3 playerMove = GetPlayerMoveDirection();
            DirectX::XMFLOAT3 playerPnt = playerAtc->getProcessingPosition();
            DirectX::XMVECTOR plyMoveVec = DirectX::XMLoadFloat3(&playerMove);
            DirectX::XMVECTOR contactVec = DirectX::XMLoadFloat3(&contactPnt) -
                DirectX::XMLoadFloat3(&playerPnt);
            contactVec = DirectX::XMVector3Normalize(contactVec);
            contactVec = DirectX::XMVectorGetX(DirectX::XMVector3Dot(
                contactVec, plyMoveVec)) * contactVec;
            DirectX::XMVECTOR moveVec = plyMoveVec - contactVec;
            float xAsix = DirectX::XMVectorGetX(DirectX::XMVector3Dot(
                moveVec, xVec));
            float zAsix = DirectX::XMVectorGetX(DirectX::XMVector3Dot(
                moveVec, zVec));
            float aimSlow = (GetPlayerAimingFlg() && GetPlayerCanAimFlg()) ?
                0.1f : 1.f;
            float deltatime = _timer.floatDeltaTime() * aimSlow;
            playerAtc->rollBackPositionX();
            playerAtc->rollBackPositionZ();
            playerAtc->translateZAsix(0.02f * deltatime * zAsix);
            playerAtc->translateXAsix(0.02f * deltatime * xAsix);
            DirectX::XMStoreFloat3(&playerMove, moveVec);
            SetPlayerMoveDirection(playerMove);
        }
    }
}

void ObstacleDestory(AInteractComponent* _aitc)
{

}
