#include "ButtonProcess.h"

#include "FadeProcess.h"

void registerButtonProcess(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getUInputMapPtr().insert(
      {FUNC_NAME(normalBtnInput), normalBtnInput});
}

static SUBMESH_DATA *G_BtnFlagSprite = nullptr;

void normalBtnInput(UInputComponent *Uic, const Timer &Timer) {
  static std::string SceneName = "";
  static std::string SceneFile = "";

  if (!G_BtnFlagSprite) {
    G_BtnFlagSprite = Uic->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
        SELECTED_BTN_SPRITE_NAME);
  }

  if (getSceneInFlg() || getSceneOutFlg()) {
    auto &Map = G_BtnFlagSprite->InstanceMap;
    for (auto &Ins : Map) {
      auto &InsData = Ins.second;
      InsData.CustomizedData1 = {1.f, 1.f, 1.f, 0.f};
      break;
    }
    return;
  } else if (getSceneOutFinish()) {
    auto &Map = G_BtnFlagSprite->InstanceMap;
    for (auto &Ins : Map) {
      auto &InsData = Ins.second;
      InsData.CustomizedData1 = {1.f, 1.f, 1.f, 0.f};
      break;
    }
  } else {
    auto &Map = G_BtnFlagSprite->InstanceMap;
    for (auto &Ins : Map) {
      auto &InsData = Ins.second;
      InsData.CustomizedData1 = {1.f, 1.f, 1.f, 1.f};
      break;
    }
  }

  if (Uic->getUiOwner()->getSceneNode().getSceneNodeName() == "title-scene") {
    auto &Map = G_BtnFlagSprite->InstanceMap;
    for (auto &Ins : Map) {
      auto &InsData = Ins.second;
      InsData.CustomizedData1 = {1.f, 1.f, 1.f, 0.f};
      break;
    }
  }

  auto Ubc = Uic->getUiOwner()->getComponent<UButtonComponent>();
  if (!Ubc) {
    return;
  }

  if (input::isKeyPushedInSingle(KB_UP)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("select-btn",
                                                               0.3f);
    Ubc->selectUpBtn();
  }
  if (input::isKeyPushedInSingle(KB_DOWN)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("select-btn",
                                                               0.3f);
    Ubc->selectDownBtn();
  }
  if (input::isKeyPushedInSingle(KB_LEFT)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("select-btn",
                                                               0.3f);
    Ubc->selectLeftBtn();
  }
  if (input::isKeyPushedInSingle(KB_RIGHT)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("select-btn",
                                                               0.3f);
    Ubc->selectRightBtn();
  }
  if (input::isKeyPushedInSingle(GP_UPDIRBTN)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("select-btn",
                                                               0.3f);
    Ubc->selectUpBtn();
  }
  if (input::isKeyPushedInSingle(GP_DOWNDIRBTN)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("select-btn",
                                                               0.3f);
    Ubc->selectDownBtn();
  }
  if (input::isKeyPushedInSingle(GP_LEFTDIRBTN)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("select-btn",
                                                               0.3f);
    Ubc->selectLeftBtn();
  }
  if (input::isKeyPushedInSingle(GP_RIGHTDIRBTN)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("select-btn",
                                                               0.3f);
    Ubc->selectRightBtn();
  }

  if ((Ubc->isCursorOnBtn() && input::isKeyPushedInSingle(M_LEFTBTN)) ||
      ((input::isKeyPushedInSingle(KB_RETURN) ||
        input::isKeyPushedInSingle(GP_BOTTOMBTN)) &&
       Ubc->isBeingSelected())) {
    if (Ubc->getCompName() == "back-title-btn-ui-button") {
      Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("click-btn",
                                                                 0.3f);
      SceneName = "title-scene";
      SceneFile = "title-scene.json";
      P_LOG(LOG_DEBUG, "to title");
      setSceneOutFlg(true);
    } else if (Ubc->getCompName() == "tutorial-btn-ui-button") {
      stopBGM();
      Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe(
          "start-tutorial", 0.3f);
      SceneName = "tutorial-scene";
      SceneFile = "tutorial-scene.json";
      P_LOG(LOG_DEBUG, "to tutorial");
      setSceneOutFlg(true);
    } else if (Ubc->getCompName() == "route1-btn-ui-button") {
      stopBGM();
      setVolume("route1", 0.2f);
      playBGM("route1");
      Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("start-run",
                                                                 0.3f);
      SceneName = "run-scene";
      SceneFile = "run-scene.json";
      P_LOG(LOG_DEBUG, "to run");
      setSceneOutFlg(true);
    } else if (Ubc->getCompName() == "route2-btn-ui-button") {
      stopBGM();
      setVolume("route2", 0.2f);
      playBGM("route2");
      Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("start-run",
                                                                 0.3f);
      SceneName = "route2-scene";
      SceneFile = "route2-scene.json";
      P_LOG(LOG_DEBUG, "to run2");
      setSceneOutFlg(true);
    } else if (Ubc->getCompName() == "quit-btn-ui-button") {
      PostQuitMessage(0);
    } else if (Ubc->getCompName() == "result-title-btn-ui-button") {
      stopBGM();
      setVolume("title", 0.2f);
      playBGM("title");
      Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("click-btn",
                                                                 0.3f);
      SceneName = "title-scene";
      SceneFile = "title-scene.json";
      P_LOG(LOG_DEBUG, "to title");
      setSceneOutFlg(true);
    } else if (Ubc->getCompName() == "start-game-btn-ui-button") {
      SceneName = "select-scene";
      SceneFile = "select-scene.json";
      P_LOG(LOG_DEBUG, "to select");
      setSceneOutFlg(true);
    }
  }

  if (getSceneOutFinish()) {
    G_BtnFlagSprite = nullptr;
    Uic->getUiOwner()->getSceneNode().getSceneManager()->loadSceneNode(
        SceneName.c_str(), SceneFile.c_str());
  }
}
