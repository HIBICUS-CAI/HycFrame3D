#include "PlayerProcess.h"
#include "WM_Interface.h"
#include <vector>

void RegisterPlayerProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(PlayerMove),PlayerMove });
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(PlayerInit),PlayerInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(PlayerUpdate),PlayerUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(PlayerDestory),PlayerDestory });
}

static float g_PlayerYAsixSpeed = 0.f;
static std::vector<std::string> g_GroundObjNameVec = {};

void PlayerMove(AInputComponent* _aic, Timer& _timer)
{
    float deltatime = _timer.FloatDeltaTime();

    auto mouseOffset = InputInterface::GetMouseOffset();
    float horiR = mouseOffset.x * deltatime / 2000.f;
    float vertR = -mouseOffset.y * deltatime / 2000.f;
    _aic->GetActorOwner()->GetSceneNode().GetMainCamera()->
        RotateRSCamera(vertR, horiR);

    DirectX::XMFLOAT3 camOffset = { 0.f,0.f,0.f };
    auto atc = _aic->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    atc->RotateYAsix(horiR);

    float cosF = cosf(atc->GetProcessingRotation().y);
    float sinF = sinf(atc->GetProcessingRotation().y);
    float cosR = cosf(atc->GetProcessingRotation().y + DirectX::XM_PIDIV2);
    float sinR = sinf(atc->GetProcessingRotation().y + DirectX::XM_PIDIV2);

    if (InputInterface::IsKeyDownInSingle(KB_W))
    {
        atc->TranslateZAsix(0.02f * deltatime * cosF);
        atc->TranslateXAsix(0.02f * deltatime * sinF);
    }
    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        atc->TranslateZAsix(0.02f * deltatime * -cosR);
        atc->TranslateXAsix(0.02f * deltatime * -sinR);
    }
    if (InputInterface::IsKeyDownInSingle(KB_S))
    {
        atc->TranslateZAsix(0.02f * deltatime * -cosF);
        atc->TranslateXAsix(0.02f * deltatime * -sinF);
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        atc->TranslateZAsix(0.02f * deltatime * cosR);
        atc->TranslateXAsix(0.02f * deltatime * sinR);
    }
    if (InputInterface::IsKeyPushedInSingle(KB_SPACE))
    {
        g_PlayerYAsixSpeed = 50.f;
    }

    g_PlayerYAsixSpeed -= 120.f * (deltatime / 1000.f);
    atc->TranslateYAsix(g_PlayerYAsixSpeed * (deltatime / 1000.f));
}

bool PlayerInit(AInteractComponent* _aitc)
{
    _aitc->GetActorOwner()->GetSceneNode().GetMainCamera()->
        RotateRSCamera(0.f, _aitc->GetActorOwner()->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            GetRotation().y);

    g_PlayerYAsixSpeed = 0.f;

    g_GroundObjNameVec.clear();
    int groundSize = 1;
    for (int i = 0; i < groundSize; i++)
    {
        g_GroundObjNameVec.push_back("ground-" + std::to_string(i) + "-actor");
    }

    ShowCursor(FALSE);

    return true;
}

void PlayerUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    auto atc = _aitc->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    DirectX::XMFLOAT3 camOffset = atc->GetProcessingPosition();
    _aitc->GetActorOwner()->GetSceneNode().GetMainCamera()->
        ChangeRSCameraPosition(camOffset);

    auto acc = _aitc->GetActorOwner()->
        GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION);
    for (auto& ground : g_GroundObjNameVec)
    {
        if (acc->CheckCollisionWith(ground))
        {
            atc->RollBackPositionY();
            g_PlayerYAsixSpeed = 0.f;
            break;
        }
    }

    if (atc->GetProcessingPosition().y < -50.f)
    {
        P_LOG(LOG_DEBUG, "to result\n");
        _aitc->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("result-scene", "result-scene.json");
    }
}

void PlayerDestory(AInteractComponent* _aitc)
{
    _aitc->GetActorOwner()->GetSceneNode().GetMainCamera()->
        ResetRSCameraRotation({ 0.f,0.f,1.f }, { 0.f,1.f,0.f });
    ShowCursor(TRUE);
    RECT wndRect = {};
    GetClientRect(WindowInterface::GetWindowPtr()->GetWndHandle(), &wndRect);
    SetCursorPos((wndRect.right - wndRect.left) / 2,
        (wndRect.bottom - wndRect.top) / 2);
}
