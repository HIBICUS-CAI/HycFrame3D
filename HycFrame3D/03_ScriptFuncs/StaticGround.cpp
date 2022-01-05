#include "StaticGround.h"
#include "PlayerProcess.h"
#include <unordered_set>

using namespace DirectX;

static std::unordered_set<AInteractComponent*> g_GroundSet = {};

void RegisterStaticGround(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(GoundInit),GoundInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(GoundUpdate),GoundUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(GoundDestory),GoundDestory });
}

bool GoundInit(AInteractComponent* _aitc)
{
    g_GroundSet.insert(_aitc);

    return true;
}

void GoundUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    auto acc = _aitc->GetActorOwner()->
        GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION);
    static CONTACT_PONT_PAIR contact = {};
    if (acc->CheckCollisionWith(PLAYER_NAME, &contact))
    {
        auto playerAtc = _aitc->GetActorOwner()->GetSceneNode().
            GetActorObject(PLAYER_NAME)->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
        auto contactPoint =
            ACollisionComponent::CalcCenterOfContact(contact);
        float xOffset = fabsf(contactPoint.x -
            playerAtc->GetProcessingPosition().x);
        float zOffset = fabsf(contactPoint.z -
            playerAtc->GetProcessingPosition().z);
        if (xOffset < 3.1f && zOffset < 3.1f &&
            (contactPoint.y - playerAtc->GetProcessingPosition().y) < -0.1f)
        {
            playerAtc->RollBackPositionY();

            XMFLOAT3 playerPnt = playerAtc->GetProcessingPosition();
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
            playerAtc->TranslateYAsix(offset);

            SetPlayerContactGround();
            auto atc = _aitc->GetActorOwner()->
                GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
            SetPlayerLastReachGround(atc);
        }
        else
        {
            playerAtc->RollBackPosition();
            SetPlayerBrokeHead();
        }
    }
}

void GoundDestory(AInteractComponent* _aitc)
{
    g_GroundSet.erase(_aitc);
}
