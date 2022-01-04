#pragma once

#include "UiAll.h"

void RegisterPauseMenu(ObjectFactory* _factory);

void PauseMenuInput(UInputComponent*, Timer&);
void PauseMenuBtnInput(UInputComponent*, Timer&);
bool PauseMenuInit(UInteractComponent*);
void PauseMenuUpdate(UInteractComponent*, Timer&);
void PauseMenuDestory(UInteractComponent*);

bool GetGamePauseFlg();
