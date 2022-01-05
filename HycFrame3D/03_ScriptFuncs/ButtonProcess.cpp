#include "ButtonProcess.h"
#include "FadeProcess.h"

void RegisterButtonProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(NormalBtnInput),NormalBtnInput });
}

static MESH_DATA* g_BtnFlagSprite = nullptr;

void NormalBtnInput(UInputComponent* _uic, Timer& _timer)
{
    static std::string sceneName = "";
    static std::string sceneFile = "";

    if (!g_BtnFlagSprite)
    {
        g_BtnFlagSprite = _uic->GetUiOwner()->GetSceneNode().GetAssetsPool()->
            GetMeshIfExisted(SELECTED_BTN_SPRITE_NAME);
    }

    if (GetSceneInFlg() || GetSceneOutFlg())
    {
        auto& map = g_BtnFlagSprite->mInstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.mCustomizedData1 = { 1.f,1.f,1.f,0.f };
            break;
        }
        return;
    }
    else if (GetSceneOutFinish())
    {
        auto& map = g_BtnFlagSprite->mInstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.mCustomizedData1 = { 1.f,1.f,1.f,0.f };
            break;
        }
    }
    else
    {
        auto& map = g_BtnFlagSprite->mInstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.mCustomizedData1 = { 1.f,1.f,1.f,1.f };
            break;
        }
    }

    auto ubc = _uic->GetUiOwner()->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc) { return; }

    if (InputInterface::IsKeyPushedInSingle(KB_UP)) { ubc->SelectUpBtn(); }
    if (InputInterface::IsKeyPushedInSingle(KB_DOWN)) { ubc->SelectDownBtn(); }
    if (InputInterface::IsKeyPushedInSingle(KB_LEFT)) { ubc->SelectLeftBtn(); }
    if (InputInterface::IsKeyPushedInSingle(KB_RIGHT)) { ubc->SelectRightBtn(); }

    if ((ubc->IsCursorOnBtn() && InputInterface::IsKeyPushedInSingle(M_LEFTBTN)) ||
        (InputInterface::IsKeyPushedInSingle(KB_RETURN) && ubc->IsBeingSelected()))
    {
        if (ubc->GetCompName() == "back-title-btn-ui-button")
        {
            sceneName = "title-scene";
            sceneFile = "title-scene.json";
            P_LOG(LOG_DEBUG, "to title\n");
            SetSceneOutFlg(true);
        }
        else if (ubc->GetCompName() == "tutorial-btn-ui-button")
        {

        }
        else if (ubc->GetCompName() == "route1-btn-ui-button")
        {
            sceneName = "run-scene";
            sceneFile = "run-scene.json";
            P_LOG(LOG_DEBUG, "to run\n");
            SetSceneOutFlg(true);
        }
        else if (ubc->GetCompName() == "route2-btn-ui-button")
        {
            sceneName = "route2-scene";
            sceneFile = "route2-scene.json";
            P_LOG(LOG_DEBUG, "to run2\n");
            SetSceneOutFlg(true);
        }
        else if (ubc->GetCompName() == "quit-btn-ui-button")
        {
            PostQuitMessage(0);
        }
    }

    if (GetSceneOutFinish())
    {
        g_BtnFlagSprite = nullptr;
        _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode(sceneName.c_str(), sceneFile.c_str());
    }
}
