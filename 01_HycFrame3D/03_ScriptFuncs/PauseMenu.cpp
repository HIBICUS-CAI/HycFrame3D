#include "PauseMenu.h"
#include "FadeProcess.h"

void RegisterPauseMenu(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getUInputMapPtr().insert(
        { FUNC_NAME(PauseMenuInput),PauseMenuInput });
    _factory->getUInputMapPtr().insert(
        { FUNC_NAME(PauseMenuBtnInput),PauseMenuBtnInput });
    _factory->getUInitMapPtr().insert(
        { FUNC_NAME(PauseMenuInit),PauseMenuInit });
    _factory->getUUpdateMapPtr().insert(
        { FUNC_NAME(PauseMenuUpdate),PauseMenuUpdate });
    _factory->getUDestoryMapPtr().insert(
        { FUNC_NAME(PauseMenuDestory),PauseMenuDestory });
}

static bool g_PauseFlg = false;
static bool g_BackTitleTrigger = false;

static USpriteComponent* g_PauseMenuUsc[3] = { nullptr };

static SUBMESH_DATA* g_BtnFlagSprite = nullptr;

static const UINT DEST_NOT_REACH = 1;

void PauseMenuInput(UInputComponent* _uic, const Timer& _timer)
{
    if (GetSceneInFlg() || GetSceneOutFlg()) { return; }

    if (input::isKeyPushedInSingle(KB_ESCAPE) ||
        input::isKeyPushedInSingle(GP_RIGHTMENUBTN))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("click-btn", 0.3f);
        g_PauseFlg = !g_PauseFlg;
        ShowCursor(g_PauseFlg ? TRUE : FALSE);
    }
}

void PauseMenuBtnInput(UInputComponent* _uic, const Timer& _timer)
{
    auto ubc = _uic->getUiOwner()->
        getComponent<UButtonComponent>();
    if (!ubc || !g_PauseFlg) { return; }

    if (input::isKeyPushedInSingle(KB_UP))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("select-btn", 0.3f);
        ubc->selectUpBtn();
    }
    if (input::isKeyPushedInSingle(KB_DOWN))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("select-btn", 0.3f);
        ubc->selectDownBtn();
    }
    if (input::isKeyPushedInSingle(GP_UPDIRBTN))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("select-btn", 0.3f);
        ubc->selectUpBtn();
    }
    if (input::isKeyPushedInSingle(GP_DOWNDIRBTN))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("select-btn", 0.3f);
        ubc->selectDownBtn();
    }

    if ((ubc->isCursorOnBtn() && input::isKeyPushedInSingle(M_LEFTBTN)) ||
        ((input::isKeyPushedInSingle(KB_RETURN) ||
            input::isKeyPushedInSingle(GP_BOTTOMBTN)) &&
            ubc->isBeingSelected()))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("click-btn", 0.3f);

        if (ubc->getCompName() == "pause-continue-btn-ui-button")
        {
            g_PauseFlg = false;
            ShowCursor(FALSE);
        }
        else if (ubc->getCompName() == "pause-title-btn-ui-button")
        {
            SetSceneOutFlg(true, DEST_NOT_REACH);
            ShowCursor(FALSE);
            auto& map = g_BtnFlagSprite->InstanceMap;
            for (auto& ins : map)
            {
                auto& ins_data = ins.second;
                ins_data.CustomizedData1 = { 1.f,1.f,1.f,0.f };
                break;
            }
        }
    }
    if (GetSceneOutFinish(DEST_NOT_REACH))
    {
        P_LOG(LOG_DEBUG, "to title\n");
        _uic->getUiOwner()->getSceneNode().getSceneManager()->
            loadSceneNode("title-scene", "title-scene.json");
        stopBGM();
        setVolume("title", 0.2f);
        playBGM("title");
    }
}

bool PauseMenuInit(UInteractComponent* _uitc)
{
    g_PauseFlg = false;
    g_BackTitleTrigger = false;

    auto compContainer = _uitc->getUiOwner()->getSceneNode().
        getComponentContainer();
    g_PauseMenuUsc[0] = (USpriteComponent*)(compContainer->
        getComponent("pause-menu-main-ui-sprite"));
    g_PauseMenuUsc[1] = (USpriteComponent*)(compContainer->
        getComponent("pause-continue-btn-ui-sprite"));
    g_PauseMenuUsc[2] = (USpriteComponent*)(compContainer->
        getComponent("pause-title-btn-ui-sprite"));

    g_BtnFlagSprite = nullptr;

    return true;
}

void PauseMenuUpdate(UInteractComponent* _uitc, const Timer& _timer)
{
    if (!g_BtnFlagSprite)
    {
        g_BtnFlagSprite = _uitc->getUiOwner()->getSceneNode().getAssetsPool()->
            getSubMeshIfExisted(SELECTED_BTN_SPRITE_NAME);
    }

    if (g_PauseFlg)
    {
        g_PauseMenuUsc[0]->setOffsetColor({ 0.f,0.f,0.f,0.8f });
        g_PauseMenuUsc[1]->setOffsetColor({ 1.f,1.f,1.f,1.f });
        g_PauseMenuUsc[2]->setOffsetColor({ 1.f,1.f,1.f,1.f });
        auto& map = g_BtnFlagSprite->InstanceMap;
        if (GetSceneOutFlg())
        {
            for (auto& ins : map)
            {
                auto& ins_data = ins.second;
                ins_data.CustomizedData1 = { 1.f,1.f,1.f,0.f };
                break;
            }
        }
        else
        {
            for (auto& ins : map)
            {
                auto& ins_data = ins.second;
                ins_data.CustomizedData1 = { 1.f,1.f,1.f,1.f };
                break;
            }
        }
    }
    else
    {
        g_PauseMenuUsc[0]->setOffsetColor({ 0.f,0.f,0.f,0.f });
        g_PauseMenuUsc[1]->setOffsetColor({ 1.f,1.f,1.f,0.f });
        g_PauseMenuUsc[2]->setOffsetColor({ 1.f,1.f,1.f,0.f });
        auto& map = g_BtnFlagSprite->InstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.CustomizedData1 = { 1.f,1.f,1.f,0.f };
            break;
        }
    }
}

void PauseMenuDestory(UInteractComponent* _uitc)
{
    g_PauseFlg = false;
    g_BackTitleTrigger = false;

    g_PauseMenuUsc[0] = nullptr;
    g_PauseMenuUsc[1] = nullptr;
    g_PauseMenuUsc[2] = nullptr;
}

bool GetGamePauseFlg()
{
    return g_PauseFlg;
}
