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

    if (_uic->GetUiOwner()->GetSceneNode().GetSceneNodeName() == "title-scene")
    {
        auto& map = g_BtnFlagSprite->mInstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.mCustomizedData1 = { 1.f,1.f,1.f,0.f };
            break;
        }
    }

    auto ubc = _uic->GetUiOwner()->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc) { return; }

    if (InputInterface::IsKeyPushedInSingle(KB_UP))
    {
        _uic->GetUiOwner()->GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlaySe("select-btn", 0.3f);
        ubc->SelectUpBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_DOWN))
    {
        _uic->GetUiOwner()->GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlaySe("select-btn", 0.3f);
        ubc->SelectDownBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_LEFT))
    {
        _uic->GetUiOwner()->GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlaySe("select-btn", 0.3f);
        ubc->SelectLeftBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_RIGHT))
    {
        _uic->GetUiOwner()->GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlaySe("select-btn", 0.3f);
        ubc->SelectRightBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(GP_UPDIRBTN))
    {
        _uic->GetUiOwner()->GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlaySe("select-btn", 0.3f);
        ubc->SelectUpBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(GP_DOWNDIRBTN))
    {
        _uic->GetUiOwner()->GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlaySe("select-btn", 0.3f);
        ubc->SelectDownBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(GP_LEFTDIRBTN))
    {
        _uic->GetUiOwner()->GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlaySe("select-btn", 0.3f);
        ubc->SelectLeftBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(GP_RIGHTDIRBTN))
    {
        _uic->GetUiOwner()->GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlaySe("select-btn", 0.3f);
        ubc->SelectRightBtn();
    }

    if ((ubc->IsCursorOnBtn() && InputInterface::IsKeyPushedInSingle(M_LEFTBTN)) ||
        ((InputInterface::IsKeyPushedInSingle(KB_RETURN) ||
            InputInterface::IsKeyPushedInSingle(GP_BOTTOMBTN)) &&
            ubc->IsBeingSelected()))
    {
        if (ubc->GetCompName() == "back-title-btn-ui-button")
        {
            _uic->GetUiOwner()->
                GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
                PlaySe("click-btn", 0.3f);
            sceneName = "title-scene";
            sceneFile = "title-scene.json";
            P_LOG(LOG_DEBUG, "to title\n");
            SetSceneOutFlg(true);
        }
        else if (ubc->GetCompName() == "tutorial-btn-ui-button")
        {
            StopBGM();
            _uic->GetUiOwner()->
                GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
                PlaySe("start-tutorial", 0.3f);
            sceneName = "tutorial-scene";
            sceneFile = "tutorial-scene.json";
            P_LOG(LOG_DEBUG, "to tutorial\n");
            SetSceneOutFlg(true);
        }
        else if (ubc->GetCompName() == "route1-btn-ui-button")
        {
            StopBGM();
            SetVolume("route1", 0.2f);
            PlayBGM("route1");
            _uic->GetUiOwner()->
                GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
                PlaySe("start-run", 0.3f);
            sceneName = "run-scene";
            sceneFile = "run-scene.json";
            P_LOG(LOG_DEBUG, "to run\n");
            SetSceneOutFlg(true);
        }
        else if (ubc->GetCompName() == "route2-btn-ui-button")
        {
            StopBGM();
            SetVolume("route2", 0.2f);
            PlayBGM("route2");
            _uic->GetUiOwner()->
                GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
                PlaySe("start-run", 0.3f);
            sceneName = "route2-scene";
            sceneFile = "route2-scene.json";
            P_LOG(LOG_DEBUG, "to run2\n");
            SetSceneOutFlg(true);
        }
        else if (ubc->GetCompName() == "quit-btn-ui-button")
        {
            PostQuitMessage(0);
        }
        else if (ubc->GetCompName() == "result-title-btn-ui-button")
        {
            StopBGM();
            SetVolume("title", 0.2f);
            PlayBGM("title");
            _uic->GetUiOwner()->
                GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
                PlaySe("click-btn", 0.3f);
            sceneName = "title-scene";
            sceneFile = "title-scene.json";
            P_LOG(LOG_DEBUG, "to title\n");
            SetSceneOutFlg(true);
        }
        else if (ubc->GetCompName() == "start-game-btn-ui-button")
        {
            sceneName = "select-scene";
            sceneFile = "select-scene.json";
            P_LOG(LOG_DEBUG, "to select\n");
            SetSceneOutFlg(true);
        }
    }

    if (GetSceneOutFinish())
    {
        g_BtnFlagSprite = nullptr;
        _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode(sceneName.c_str(), sceneFile.c_str());
    }
}
