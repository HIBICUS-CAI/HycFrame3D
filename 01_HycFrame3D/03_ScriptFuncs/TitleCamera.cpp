#include "TitleCamera.h"

void RegisterTitleCamera(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(TitleCamInit),TitleCamInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(TitleCamUpdate),TitleCamUpdate });
    _factory->getADestoryMapPtr().insert(
        { FUNC_NAME(TitleCamDestory),TitleCamDestory });
}

static RSCamera* g_MainCam = nullptr;

bool TitleCamInit(AInteractComponent* _aitc)
{
    g_MainCam = _aitc->getActorOwner()->getSceneNode().getMainCamera();
    if (!g_MainCam) { return false; }
    g_MainCam->rotateRSCamera(0.25f, 0.f);

    return true;
}

void TitleCamUpdate(AInteractComponent* _aitc, const Timer& _timer)
{
    float time = 0.2f * _timer.floatDeltaTime() / 1000.f;
    g_MainCam->rotateRSCamera(0.f, time);
}

void TitleCamDestory(AInteractComponent* _aitc)
{
    g_MainCam->resetRSCameraRotation({ 0.f,0.f,1.f }, { 0.f,1.f,0.f });
    g_MainCam = nullptr;
}
