#include "GM02.h"
#include "RSCamera.h"

void RegisterGM02(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(PlayerInput),PlayerInput });
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(PlayerInit),PlayerInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(PlayerUpdate),PlayerUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(PlayerDestory),PlayerDestory });
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(BulletInit),BulletInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(BulletUpdate),BulletUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(BulletDestory),BulletDestory });
}

static ATransformComponent* g_PlayerAtc = nullptr;
static RSCamera* g_Cam = nullptr;

void CreateBullet(DirectX::XMFLOAT3 _startPos, SceneNode& _scene);

void PlayerInput(AInputComponent* _aic, Timer& _timer)
{
    float deltatime = _timer.FloatDeltaTime();

    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        g_PlayerAtc->TranslateXAsix(-0.2f * deltatime);
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        g_PlayerAtc->TranslateXAsix(0.2f * deltatime);
    }

    if (fabsf(g_PlayerAtc->GetProcessingPosition().x) > 80.f)
    {
        g_PlayerAtc->RollBackPositionX();
    }

    if (InputInterface::IsKeyPushedInSingle(KB_SPACE))
    {
        CreateBullet(g_PlayerAtc->GetProcessingPosition(),
            _aic->GetActorOwner()->GetSceneNode());
    }
}

bool PlayerInit(AInteractComponent* _aitc)
{
    g_PlayerAtc = _aitc->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    if (!g_PlayerAtc) { return false; }
    g_Cam = _aitc->GetActorOwner()->GetSceneNode().GetMainCamera();
    if (!g_Cam) { return false; }

    return true;
}

void PlayerUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    using namespace DirectX;
    XMVECTOR processing = DirectX::XMLoadFloat3(
        &g_PlayerAtc->GetProcessingPosition());
    XMVECTOR origin = DirectX::XMLoadFloat3(
        &g_PlayerAtc->GetPosition());
    XMFLOAT3 deltaPos = {};
    XMStoreFloat3(&deltaPos, origin - processing);
    g_Cam->TranslateRSCamera(deltaPos);
}

void PlayerDestory(AInteractComponent* _aitc)
{
    g_PlayerAtc = nullptr;
    g_Cam = nullptr;
}

void CreateBullet(DirectX::XMFLOAT3 _startPos, SceneNode& _scene)
{
    static unsigned long long int index = 0;
    std::string actorName = "bullet-actor" + std::to_string(index++);
    ActorObject bulletActor(actorName, _scene);

    {
        ATransformComponent atc(actorName + "-transform", nullptr);
        _startPos.y += 10.f;
        atc.ForcePosition(_startPos);
        atc.ForceScaling({ 0.2f,0.2f,0.2f });
        bulletActor.AddAComponent(COMP_TYPE::A_TRANSFORM);
        _scene.GetComponentContainer()->AddComponent(
            COMP_TYPE::A_TRANSFORM, atc);
    }

    {
        AMeshComponent amc(actorName + "-mesh", nullptr);
        amc.AddMeshInfo("sphere");
        bulletActor.AddAComponent(COMP_TYPE::A_MESH);
        _scene.GetComponentContainer()->AddComponent(
            COMP_TYPE::A_MESH, amc);
    }

    {
        ATimerComponent atmc(actorName + "-timer", nullptr);
        atmc.AddTimer("age");
        bulletActor.AddAComponent(COMP_TYPE::A_TIMER);
        _scene.GetComponentContainer()->AddComponent(
            COMP_TYPE::A_TIMER, atmc);
    }

    {
        AInteractComponent aitc(actorName + "-interact", nullptr);
        aitc.SetInitFunction(BulletInit);
        aitc.SetUpdateFunction(BulletUpdate);
        aitc.SetDestoryFunction(BulletDestory);
        bulletActor.AddAComponent(COMP_TYPE::A_INTERACT);
        _scene.GetComponentContainer()->AddComponent(
            COMP_TYPE::A_INTERACT, aitc);
    }

    _scene.AddActorObject(bulletActor);
}

bool BulletInit(AInteractComponent* _aitc)
{
    _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        StartTimer("age");
    return true;
}

void BulletUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    auto atmc = _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER);
    if (atmc->GetTimer("age")->IsGreaterThan(1.f))
    {
        _aitc->GetActorOwner()->SetObjectStatus(STATUS::NEED_DESTORY);
        _aitc->GetActorOwner()->GetSceneNode().GetObjectContainer()->
            DeleteActorObject(const_cast<std::string&>(_aitc->
                GetActorOwner()->GetObjectName()));
        return;
    }

    auto atc = _aitc->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    atc->TranslateZAsix(0.1f * _timer.FloatDeltaTime());
}

void BulletDestory(AInteractComponent* _aitc)
{

}
