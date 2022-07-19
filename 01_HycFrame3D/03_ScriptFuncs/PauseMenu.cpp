#include "PauseMenu.h"

#include "FadeProcess.h"

void registerPauseMenu(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getUInputMapPtr().insert(
      {FUNC_NAME(pauseMenuInput), pauseMenuInput});
  Factory->getUInputMapPtr().insert(
      {FUNC_NAME(pauseMenuBtnInput), pauseMenuBtnInput});
  Factory->getUInitMapPtr().insert({FUNC_NAME(pauseMenuInit), pauseMenuInit});
  Factory->getUUpdateMapPtr().insert(
      {FUNC_NAME(pauseMenuUpdate), pauseMenuUpdate});
  Factory->getUDestoryMapPtr().insert(
      {FUNC_NAME(pauseMenuDestory), pauseMenuDestory});
}

static bool G_PauseFlg = false;
static bool G_BackTitleTrigger = false;

static USpriteComponent *G_PauseMenuUsc[3] = {nullptr};

static SUBMESH_DATA *G_BtnFlagSprite = nullptr;

static const UINT DEST_NOT_REACH = 1;

void pauseMenuInput(UInputComponent *Uic, const Timer &Timer) {
  if (getSceneInFlg() || getSceneOutFlg()) {
    return;
  }

  if (input::isKeyPushedInSingle(KB_ESCAPE) ||
      input::isKeyPushedInSingle(GP_RIGHTMENUBTN)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("click-btn",
                                                               0.3f);
    G_PauseFlg = !G_PauseFlg;
    ShowCursor(G_PauseFlg ? TRUE : FALSE);
  }
}

void pauseMenuBtnInput(UInputComponent *Uic, const Timer &Timer) {
  auto Ubc = Uic->getUiOwner()->getComponent<UButtonComponent>();
  if (!Ubc || !G_PauseFlg) {
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

  if ((Ubc->isCursorOnBtn() && input::isKeyPushedInSingle(M_LEFTBTN)) ||
      ((input::isKeyPushedInSingle(KB_RETURN) ||
        input::isKeyPushedInSingle(GP_BOTTOMBTN)) &&
       Ubc->isBeingSelected())) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playSe("click-btn",
                                                               0.3f);

    if (Ubc->getCompName() == "pause-continue-btn-ui-button") {
      G_PauseFlg = false;
      ShowCursor(FALSE);
    } else if (Ubc->getCompName() == "pause-title-btn-ui-button") {
      setSceneOutFlg(true, DEST_NOT_REACH);
      ShowCursor(FALSE);
      auto &Map = G_BtnFlagSprite->InstanceMap;
      for (auto &Ins : Map) {
        auto &InsData = Ins.second;
        InsData.CustomizedData1 = {1.f, 1.f, 1.f, 0.f};
        break;
      }
    }
  }
  if (getSceneOutFinish(DEST_NOT_REACH)) {
    P_LOG(LOG_DEBUG, "to title");
    Uic->getSceneNode().getSceneManager()->loadSceneNode("title-scene",
                                                         "title-scene.json");
    stopBGM();
    setVolume("title", 0.2f);
    playBGM("title");
  }
}

bool pauseMenuInit(UInteractComponent *Uitc) {
  G_PauseFlg = false;
  G_BackTitleTrigger = false;

  auto CompContainer = Uitc->getSceneNode().getComponentContainer();
  G_PauseMenuUsc[0] = static_cast<USpriteComponent *>(
      CompContainer->getComponent("pause-menu-main-ui-sprite"));
  G_PauseMenuUsc[1] = static_cast<USpriteComponent *>(
      CompContainer->getComponent("pause-continue-btn-ui-sprite"));
  G_PauseMenuUsc[2] = static_cast<USpriteComponent *>(
      CompContainer->getComponent("pause-title-btn-ui-sprite"));

  G_BtnFlagSprite = nullptr;

  return true;
}

void pauseMenuUpdate(UInteractComponent *Uitc, const Timer &Timer) {
  if (!G_BtnFlagSprite) {
    G_BtnFlagSprite = Uitc->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
        SELECTED_BTN_SPRITE_NAME);
  }

  if (G_PauseFlg) {
    G_PauseMenuUsc[0]->setOffsetColor({0.f, 0.f, 0.f, 0.8f});
    G_PauseMenuUsc[1]->setOffsetColor({1.f, 1.f, 1.f, 1.f});
    G_PauseMenuUsc[2]->setOffsetColor({1.f, 1.f, 1.f, 1.f});
    auto &Map = G_BtnFlagSprite->InstanceMap;
    if (getSceneOutFlg()) {
      for (auto &Ins : Map) {
        auto &InsData = Ins.second;
        InsData.CustomizedData1 = {1.f, 1.f, 1.f, 0.f};
        break;
      }
    } else {
      for (auto &Ins : Map) {
        auto &InsData = Ins.second;
        InsData.CustomizedData1 = {1.f, 1.f, 1.f, 1.f};
        break;
      }
    }
  } else {
    G_PauseMenuUsc[0]->setOffsetColor({0.f, 0.f, 0.f, 0.f});
    G_PauseMenuUsc[1]->setOffsetColor({1.f, 1.f, 1.f, 0.f});
    G_PauseMenuUsc[2]->setOffsetColor({1.f, 1.f, 1.f, 0.f});
    auto &Map = G_BtnFlagSprite->InstanceMap;
    for (auto &Ins : Map) {
      auto &InsData = Ins.second;
      InsData.CustomizedData1 = {1.f, 1.f, 1.f, 0.f};
      break;
    }
  }
}

void pauseMenuDestory(UInteractComponent *Uitc) {
  G_PauseFlg = false;
  G_BackTitleTrigger = false;

  G_PauseMenuUsc[0] = nullptr;
  G_PauseMenuUsc[1] = nullptr;
  G_PauseMenuUsc[2] = nullptr;
}

bool getGamePauseFlg() { return G_PauseFlg; }
