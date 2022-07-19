#pragma once

#include "UiAll.h"

void registerPauseMenu(ObjectFactory *Factory);

void pauseMenuInput(UInputComponent *, const Timer &);
void pauseMenuBtnInput(UInputComponent *, const Timer &);
bool pauseMenuInit(UInteractComponent *);
void pauseMenuUpdate(UInteractComponent *, const Timer &);
void pauseMenuDestory(UInteractComponent *);

bool getGamePauseFlg();
