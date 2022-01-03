#include "StaticGround.h"
#include "PlayerProcess.h"
#include <unordered_set>

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
        if (fabsf(contactPoint.x - playerAtc->GetProcessingPosition().x) < 1.f &&
            fabsf(contactPoint.z - playerAtc->GetProcessingPosition().z) < 1.f &&
            (contactPoint.y - playerAtc->GetProcessingPosition().y) < 3.1f)
        {
            playerAtc->RollBackPositionY();
            SetPlayerContactGround();
        }
        else if ((contactPoint.y - playerAtc->GetProcessingPosition().y) > 0.f)
        {
            playerAtc->RollBackPositionY();
            SetPlayerBrokeHead();
        }
    }
}

void GoundDestory(AInteractComponent* _aitc)
{
    g_GroundSet.erase(_aitc);
}
