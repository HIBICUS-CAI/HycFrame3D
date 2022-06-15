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
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(EffectInit),EffectInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(EffectUpdate),EffectUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(EffectDestory),EffectDestory });
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(ButtonInput),ButtonInput });
}

static ATransformComponent* g_PlayerAtc = nullptr;
static RSCamera* g_Cam = nullptr;
static std::unordered_map<std::string, DirectX::XMFLOAT3> g_BulletDirMap = {};

void CreateBullet(DirectX::XMFLOAT3 _startPos, DirectX::XMFLOAT3 _dirVec,
    SceneNode& _scene);

void CreateBillboard(DirectX::XMFLOAT3 _startPos, SceneNode& _scene);

void PlayerInput(AInputComponent* _aic, Timer& _timer)
{
    float deltatime = _timer.FloatDeltaTime();

    if (InputInterface::IsKeyDownInSingle(KB_Q))
    {
        g_PlayerAtc->RotateYAsix(-0.005f * deltatime);
    }
    if (InputInterface::IsKeyDownInSingle(KB_E))
    {
        g_PlayerAtc->RotateYAsix(0.005f * deltatime);
    }
    float angle = g_PlayerAtc->GetProcessingRotation().y - 3.14f;
    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        g_PlayerAtc->TranslateXAsix(-0.2f * deltatime * cosf(angle));
        g_PlayerAtc->TranslateZAsix(0.2f * deltatime * sinf(angle));
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        g_PlayerAtc->TranslateXAsix(0.2f * deltatime * cosf(angle));
        g_PlayerAtc->TranslateZAsix(-0.2f * deltatime * sinf(angle));
    }
    if (InputInterface::IsKeyDownInSingle(KB_W))
    {
        g_PlayerAtc->TranslateXAsix(0.2f * deltatime * sinf(angle));
        g_PlayerAtc->TranslateZAsix(0.2f * deltatime * cosf(angle));
    }
    if (InputInterface::IsKeyDownInSingle(KB_S))
    {
        g_PlayerAtc->TranslateXAsix(-0.2f * deltatime * sinf(angle));
        g_PlayerAtc->TranslateZAsix(-0.2f * deltatime * cosf(angle));
    }

    if (fabsf(g_PlayerAtc->GetProcessingPosition().x) > 80.f)
    {
        g_PlayerAtc->RollBackPositionX();
    }
    if (fabsf(g_PlayerAtc->GetProcessingPosition().z - 90.f) > 80.f)
    {
        g_PlayerAtc->RollBackPositionZ();
    }

    if (InputInterface::IsKeyPushedInSingle(KB_SPACE))
    {
        DirectX::XMFLOAT3 dirVec = g_PlayerAtc->GetProcessingRotation();
        dirVec.y -= 3.14f;
        dirVec.x = sinf(dirVec.y);
        dirVec.z = cosf(dirVec.y);
        dirVec.y = 0.f;
        CreateBullet(g_PlayerAtc->GetProcessingPosition(), dirVec,
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
    g_Cam->ResetRSCameraRotation({ 0.f,0.f,1.f }, { 0.f,1.f,0.f });

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
    deltaPos.z *= -1.f;
    g_Cam->TranslateRSCamera(deltaPos);

    float theta = g_PlayerAtc->GetProcessingRotation().y - 3.14f;
    g_Cam->ResetRSCameraRotation(
        { sinf(theta),0.f,cosf(theta) }, { 0.f,1.f,0.f });
    float newX = -50.f * sinf(theta), newZ = -50.f * cosf(theta);
    DirectX::XMFLOAT3 now = g_PlayerAtc->GetProcessingPosition();
    now.x += newX; now.y = 0.f; now.z += newZ;
    g_Cam->ChangeRSCameraPosition(now);
}

void PlayerDestory(AInteractComponent* _aitc)
{
    g_PlayerAtc = nullptr;
    g_Cam = nullptr;
    g_BulletDirMap.clear();
}

void CreateBullet(DirectX::XMFLOAT3 _startPos, DirectX::XMFLOAT3 _dirVec,
    SceneNode& _scene)
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
        g_BulletDirMap.insert({ aitc.GetCompName(),_dirVec });
    }

    _scene.AddActorObject(bulletActor);
}

void CreateBillboard(DirectX::XMFLOAT3 _startPos, SceneNode& _scene)
{
    static unsigned long long int index = 0;
    std::string actorName = "billboard-actor" + std::to_string(index++);
    ActorObject bbActor(actorName, _scene);

    {
        ATransformComponent atc(actorName + "-transform", nullptr);
        atc.ForcePosition(_startPos);
        atc.ForceScaling({ 1.f,1.f,1.f });
        bbActor.AddAComponent(COMP_TYPE::A_TRANSFORM);
        _scene.GetComponentContainer()->AddComponent(
            COMP_TYPE::A_TRANSFORM, atc);
    }

    {
        ASpriteComponent asc(actorName + "-sprite", nullptr);
        asc.SetSpriteProperty({ 25.f,25.f }, {}, true);
        asc.SetAnimationProperty({ 0.2f,0.2f }, 25, true, 0.05f);
        asc.CreateGeoPointWithTexture(&_scene, "boom.png");
        bbActor.AddAComponent(COMP_TYPE::A_SPRITE);
        _scene.GetComponentContainer()->AddComponent(
            COMP_TYPE::A_SPRITE, asc);
    }

    {
        ATimerComponent atmc(actorName + "-timer", nullptr);
        atmc.AddTimer("age");
        bbActor.AddAComponent(COMP_TYPE::A_TIMER);
        _scene.GetComponentContainer()->AddComponent(
            COMP_TYPE::A_TIMER, atmc);
    }

    {
        AInteractComponent aitc(actorName + "-interact", nullptr);
        aitc.SetInitFunction(EffectInit);
        aitc.SetUpdateFunction(EffectUpdate);
        aitc.SetDestoryFunction(EffectDestory);
        bbActor.AddAComponent(COMP_TYPE::A_INTERACT);
        _scene.GetComponentContainer()->AddComponent(
            COMP_TYPE::A_INTERACT, aitc);
    }

    _scene.AddActorObject(bbActor);
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
    using namespace DirectX;
    XMFLOAT3 dir = g_BulletDirMap[_aitc->GetCompName()];
    XMVECTOR dirVec = XMLoadFloat3(&dir);
    dirVec = XMVector3Normalize(dirVec) * 0.1f;
    DirectX::XMStoreFloat3(&dir, dirVec);
    atc->Translate(dir);

    auto acc = _aitc->GetActorOwner()->
        GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION);
    auto e0 = _aitc->GetActorOwner()->
        GetSceneNode().GetActorObject("enemy01-actor");
    auto e1 = _aitc->GetActorOwner()->
        GetSceneNode().GetActorObject("enemy02-actor");

    CONTACT_PONT_PAIR p = {};
    if (e0 && acc->CheckCollisionWith("enemy01-actor", &p))
    {
        auto effectPoint = ACollisionComponent::CalcCenterOfContact(p);
        CreateBillboard(effectPoint, _aitc->GetActorOwner()->GetSceneNode());
        _aitc->GetActorOwner()->SetObjectStatus(STATUS::NEED_DESTORY);
        _aitc->GetActorOwner()->GetSceneNode().GetObjectContainer()->
            DeleteActorObject(const_cast<std::string&>(_aitc->
                GetActorOwner()->GetObjectName()));
        e0->SetObjectStatus(STATUS::NEED_DESTORY);
        e0->GetSceneNode().GetObjectContainer()->
            DeleteActorObject(const_cast<std::string&>(e0->GetObjectName()));
    }
    else if (e1 && acc->CheckCollisionWith("enemy02-actor", &p))
    {
        auto effectPoint = ACollisionComponent::CalcCenterOfContact(p);
        CreateBillboard(effectPoint, _aitc->GetActorOwner()->GetSceneNode());
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
    g_BulletDirMap.erase(_aitc->GetCompName());
}

bool EffectInit(AInteractComponent* _aitc)
{
    _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        StartTimer("age");
    return true;
}

void EffectUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    if (_aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        GetTimer("age")->IsGreaterThan(1.f))
    {
        _aitc->GetActorOwner()->SetObjectStatus(STATUS::NEED_DESTORY);
        _aitc->GetActorOwner()->GetSceneNode().GetObjectContainer()->
            DeleteActorObject(const_cast<std::string&>(_aitc->
                GetActorOwner()->GetObjectName()));
    }
}

void EffectDestory(AInteractComponent* _aitc)
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
