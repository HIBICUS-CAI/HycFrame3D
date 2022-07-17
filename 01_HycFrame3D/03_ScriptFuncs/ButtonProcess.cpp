#include "ButtonProcess.h"
#include "FadeProcess.h"

void RegisterButtonProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getUInputMapPtr().insert(
        { FUNC_NAME(NormalBtnInput),NormalBtnInput });
}

static SUBMESH_DATA* g_BtnFlagSprite = nullptr;

void NormalBtnInput(UInputComponent* _uic, const Timer& _timer)
{
    static std::string sceneName = "";
    static std::string sceneFile = "";

    if (!g_BtnFlagSprite)
    {
        g_BtnFlagSprite = _uic->getUiOwner()->getSceneNode().getAssetsPool()->
            getSubMeshIfExisted(SELECTED_BTN_SPRITE_NAME);
    }

    if (GetSceneInFlg() || GetSceneOutFlg())
    {
        auto& map = g_BtnFlagSprite->InstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.CustomizedData1 = { 1.f,1.f,1.f,0.f };
            break;
        }
        return;
    }
    else if (GetSceneOutFinish())
    {
        auto& map = g_BtnFlagSprite->InstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.CustomizedData1 = { 1.f,1.f,1.f,0.f };
            break;
        }
    }
    else
    {
        auto& map = g_BtnFlagSprite->InstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.CustomizedData1 = { 1.f,1.f,1.f,1.f };
            break;
        }
    }

    if (_uic->getUiOwner()->getSceneNode().getSceneNodeName() == "title-scene")
    {
        auto& map = g_BtnFlagSprite->InstanceMap;
        for (auto& ins : map)
        {
            auto& ins_data = ins.second;
            ins_data.CustomizedData1 = { 1.f,1.f,1.f,0.f };
            break;
        }
    }

    auto ubc = _uic->getUiOwner()->
        getComponent<UButtonComponent>();
    if (!ubc) { return; }

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
    if (input::isKeyPushedInSingle(KB_LEFT))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("select-btn", 0.3f);
        ubc->selectLeftBtn();
    }
    if (input::isKeyPushedInSingle(KB_RIGHT))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("select-btn", 0.3f);
        ubc->selectRightBtn();
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
    if (input::isKeyPushedInSingle(GP_LEFTDIRBTN))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("select-btn", 0.3f);
        ubc->selectLeftBtn();
    }
    if (input::isKeyPushedInSingle(GP_RIGHTDIRBTN))
    {
        _uic->getUiOwner()->getComponent<UAudioComponent>()->
            playSe("select-btn", 0.3f);
        ubc->selectRightBtn();
    }

    if ((ubc->isCursorOnBtn() && input::isKeyPushedInSingle(M_LEFTBTN)) ||
        ((input::isKeyPushedInSingle(KB_RETURN) ||
          input::isKeyPushedInSingle(GP_BOTTOMBTN)) &&
            ubc->isBeingSelected()))
    {
        if (ubc->getCompName() == "back-title-btn-ui-button")
        {
            _uic->getUiOwner()->
                getComponent<UAudioComponent>()->
                playSe("click-btn", 0.3f);
            sceneName = "title-scene";
            sceneFile = "title-scene.json";
            P_LOG(LOG_DEBUG, "to title");
            SetSceneOutFlg(true);
        }
        else if (ubc->getCompName() == "tutorial-btn-ui-button")
        {
            stopBGM();
            _uic->getUiOwner()->
                getComponent<UAudioComponent>()->
                playSe("start-tutorial", 0.3f);
            sceneName = "tutorial-scene";
            sceneFile = "tutorial-scene.json";
            P_LOG(LOG_DEBUG, "to tutorial");
            SetSceneOutFlg(true);
        }
        else if (ubc->getCompName() == "route1-btn-ui-button")
        {
            stopBGM();
            setVolume("route1", 0.2f);
            playBGM("route1");
            _uic->getUiOwner()->
                getComponent<UAudioComponent>()->
                playSe("start-run", 0.3f);
            sceneName = "run-scene";
            sceneFile = "run-scene.json";
            P_LOG(LOG_DEBUG, "to run");
            SetSceneOutFlg(true);
        }
        else if (ubc->getCompName() == "route2-btn-ui-button")
        {
            stopBGM();
            setVolume("route2", 0.2f);
            playBGM("route2");
            _uic->getUiOwner()->
                getComponent<UAudioComponent>()->
                playSe("start-run", 0.3f);
            sceneName = "route2-scene";
            sceneFile = "route2-scene.json";
            P_LOG(LOG_DEBUG, "to run2");
            SetSceneOutFlg(true);
        }
        else if (ubc->getCompName() == "quit-btn-ui-button")
        {
            PostQuitMessage(0);
        }
        else if (ubc->getCompName() == "result-title-btn-ui-button")
        {
            stopBGM();
            setVolume("title", 0.2f);
            playBGM("title");
            _uic->getUiOwner()->
                getComponent<UAudioComponent>()->
                playSe("click-btn", 0.3f);
            sceneName = "title-scene";
            sceneFile = "title-scene.json";
            P_LOG(LOG_DEBUG, "to title");
            SetSceneOutFlg(true);
        }
        else if (ubc->getCompName() == "start-game-btn-ui-button")
        {
            sceneName = "select-scene";
            sceneFile = "select-scene.json";
            P_LOG(LOG_DEBUG, "to select");
            SetSceneOutFlg(true);
        }
    }

    if (GetSceneOutFinish())
    {
        g_BtnFlagSprite = nullptr;
        _uic->getUiOwner()->getSceneNode().getSceneManager()->
            loadSceneNode(sceneName.c_str(), sceneFile.c_str());
    }
}
