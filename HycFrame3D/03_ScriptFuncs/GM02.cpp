#include "GM02.h"
#include "RSRoot_DX11.h"
#include "RSPipelinesManager.h"
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
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(ButtonInput),ButtonInput });
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
    g_Cam->ChangeRSCameraPosition({ 0.f,0.f,0.f });

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
        ACollisionComponent acc(actorName + "-collision", nullptr);
        acc.CreateCollisionShape(COLLISION_SHAPE::SPHERE,
            { 1.6f,0.f,0.f });
        bulletActor.AddAComponent(COMP_TYPE::A_COLLISION);
        _scene.GetComponentContainer()->AddComponent(
            COMP_TYPE::A_COLLISION, acc);
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

    auto acc = _aitc->GetActorOwner()->
        GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION);
    auto e0 = _aitc->GetActorOwner()->
        GetSceneNode().GetActorObject("enemy01-actor");
    auto e1 = _aitc->GetActorOwner()->
        GetSceneNode().GetActorObject("enemy02-actor");

    if (e0 && acc->CheckCollisionWith("enemy01-actor"))
    {
        _aitc->GetActorOwner()->SetObjectStatus(STATUS::NEED_DESTORY);
        _aitc->GetActorOwner()->GetSceneNode().GetObjectContainer()->
            DeleteActorObject(const_cast<std::string&>(_aitc->
                GetActorOwner()->GetObjectName()));
        e0->SetObjectStatus(STATUS::NEED_DESTORY);
        e0->GetSceneNode().GetObjectContainer()->
            DeleteActorObject(const_cast<std::string&>(e0->GetObjectName()));
    }
    else if (e1 && acc->CheckCollisionWith("enemy02-actor"))
    {
        _aitc->GetActorOwner()->SetObjectStatus(STATUS::NEED_DESTORY);
        _aitc->GetActorOwner()->GetSceneNode().GetObjectContainer()->
            DeleteActorObject(const_cast<std::string&>(_aitc->
                GetActorOwner()->GetObjectName()));
        e1->SetObjectStatus(STATUS::NEED_DESTORY);
        e1->GetSceneNode().GetObjectContainer()->
            DeleteActorObject(const_cast<std::string&>(e1->GetObjectName()));
    }
}

void BulletDestory(AInteractComponent* _aitc)
{

}

void ButtonInput(UInputComponent* _uic, Timer& _timer)
{
    auto ubc = _uic->GetUiOwner()->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc) { return; }

    if (InputInterface::IsKeyPushedInSingle(KB_UP))
    {
        ubc->SelectUpBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_LEFT))
    {
        ubc->SelectLeftBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_DOWN))
    {
        ubc->SelectDownBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_RIGHT))
    {
        ubc->SelectRightBtn();
    }

    static bool simp = true;
    static std::string basic = "light-pipeline";
    static std::string simple = "simple-pipeline";

    if (ubc->IsCursorOnBtn() && InputInterface::IsKeyPushedInSingle(M_LEFTBTN))
    {
        auto& btnName = ubc->GetCompName();
        if (btnName == "light-ui-button")
        {
            std::string target = "";
            if (simp) { target = basic; }
            else { target = simple; }
            simp = !simp;
            GetRSRoot_DX11_Singleton()->PipelinesManager()->
                SetPipeline(target);
        }
        else if (btnName == "reload-ui-button")
        {
            _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
                LoadSceneNode("gm02-scene", "gm02-scene.json");
        }
        else if (btnName == "exit-ui-button")
        {
            PostQuitMessage(0);
        }
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_RETURN) &&
        ubc->IsBeingSelected())
    {
        auto& btnName = ubc->GetCompName();
        if (btnName == "light-ui-button")
        {
            std::string target = "";
            if (simp) { target = basic; }
            else { target = simple; }
            simp = !simp;
            GetRSRoot_DX11_Singleton()->PipelinesManager()->
                SetPipeline(target);
        }
        else if (btnName == "reload-ui-button")
        {
            _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
                LoadSceneNode("gm02-scene", "gm02-scene.json");
        }
        else if (btnName == "exit-ui-button")
        {
            PostQuitMessage(0);
        }
    }
}
