#include "PauseMenu.h"
#include "FadeProcess.h"

void RegisterPauseMenu(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(PauseMenuInput),PauseMenuInput });
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(PauseMenuBtnInput),PauseMenuBtnInput });
    _factory->GetUInitMapPtr()->insert(
        { FUNC_NAME(PauseMenuInit),PauseMenuInit });
    _factory->GetUUpdateMapPtr()->insert(
        { FUNC_NAME(PauseMenuUpdate),PauseMenuUpdate });
    _factory->GetUDestoryMapPtr()->insert(
        { FUNC_NAME(PauseMenuDestory),PauseMenuDestory });
}

static bool g_PauseFlg = false;
static bool g_BackTitleTrigger = false;

static USpriteComponent* g_PauseMenuUsc[3] = { nullptr };

static MESH_DATA* g_BtnFlagSprite = nullptr;

static bool g_SceneBackTitleTrigger = false;

void PauseMenuInput(UInputComponent* _uic, Timer& _timer)
{
    if (GetSceneInFlg() || GetSceneOutFlg()) { return; }

    if (InputInterface::IsKeyPushedInSingle(KB_ESCAPE))
    {
        g_PauseFlg = !g_PauseFlg;
        ShowCursor(g_PauseFlg ? TRUE : FALSE);
    }
}

void PauseMenuBtnInput(UInputComponent* _uic, Timer& _timer)
{
    auto ubc = _uic->GetUiOwner()->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc) { return; }

    if (InputInterface::IsKeyPushedInSingle(KB_UP)) { ubc->SelectUpBtn(); }
    if (InputInterface::IsKeyPushedInSingle(KB_DOWN)) { ubc->SelectDownBtn(); }

    if ((ubc->IsCursorOnBtn() && InputInterface::IsKeyPushedInSingle(M_LEFTBTN)) ||
        (InputInterface::IsKeyPushedInSingle(KB_RETURN) && ubc->IsBeingSelected()))
    {
        if (ubc->GetCompName() == "pause-continue-btn-ui-button")
        {
            g_PauseFlg = false;
            ShowCursor(FALSE);
        }
        else if (ubc->GetCompName() == "pause-title-btn-ui-button")
        {
            g_SceneBackTitleTrigger = true;
            SetSceneOutFlg(true);
            ShowCursor(FALSE);
            auto& map = g_BtnFlagSprite->mInstanceMap;
            for (auto& ins : map)
            {
                auto& ins_data = ins.second;
                ins_data.mCustomizedData1 = { 1.f,1.f,1.f,0.f };
                break;
            }
        }
    }
    if (g_SceneBackTitleTrigger && !GetSceneOutFlg())
    {
        P_LOG(LOG_DEBUG, "to title\n");
        _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("title-scene", "title-scene.json");
    }
}

bool PauseMenuInit(UInteractComponent* _uitc)
{
    g_PauseFlg = false;
    g_BackTitleTrigger = false;

    auto compContainer = _uitc->GetUiOwner()->GetSceneNode().
        GetComponentContainer();
    g_PauseMenuUsc[0] = (USpriteComponent*)(compContainer->
        GetComponent("pause-menu-main-ui-sprite"));
    g_PauseMenuUsc[1] = (USpriteComponent*)(compContainer->
        GetComponent("pause-continue-btn-ui-sprite"));
    g_PauseMenuUsc[2] = (USpriteComponent*)(compContainer->
        GetComponent("pause-title-btn-ui-sprite"));

    g_BtnFlagSprite = nullptr;

    g_SceneBackTitleTrigger = false;

    return true;
}

void PauseMenuUpdate(UInteractComponent* _uitc, Timer& _timer)
{
    if (!g_BtnFlagSprite)
    {
        g_BtnFlagSprite = _uitc->GetUiOwner()->GetSceneNode().GetAssetsPool()->
            GetMeshIfExisted(SELECTED_BTN_SPRITE_NAME);
    }

    if (g_PauseFlg)
    {
        g_PauseMenuUsc[0]->SetOffsetColor({ 0.f,0.f,0.f,0.8f });
        g_PauseMenuUsc[1]->SetOffsetColor({ 1.f,1.f,1.f,1.f });
        g_PauseMenuUsc[2]->SetOffsetColor({ 1.f,1.f,1.f,1.f });
        auto& map = g_BtnFlagSprite->mInstanceMap;
        if (GetSceneOutFlg())
        {
            for (auto& ins : map)
            {
                auto& ins_data = ins.second;
                ins_data.mCustomizedData1 = { 1.f,1.f,1.f,0.f };
                break;
            }
        }
        else
        {
            for (auto& ins : map)
            {
                auto& ins_data = ins.second;
                ins_data.mCustomizedData1 = { 1.f,1.f,1.f,1.f };
                break;
            }
        }
    }
    else
    {
        g_PauseMenuUsc[0]->SetOffsetColor({ 0.f,0.f,0.f,0.f });
        g_PauseMenuUsc[1]->SetOffsetColor({ 1.f,1.f,1.f,0.f });
        g_PauseMenuUsc[2]->SetOffsetColor({ 1.f,1.f,1.f,0.f });
        auto& map = g_BtnFlagSprite->mInstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.mCustomizedData1 = { 1.f,1.f,1.f,0.f };
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
