#pragma once

#include "UiAll.h"

void RegisterPauseMenu(ObjectFactory* _factory);

void PauseMenuInput(UInputComponent*, const Timer&);
void PauseMenuBtnInput(UInputComponent*, const Timer&);
bool PauseMenuInit(UInteractComponent*);
void PauseMenuUpdate(UInteractComponent*, const Timer&);
void PauseMenuDestory(UInteractComponent*);

bool GetGamePauseFlg();
