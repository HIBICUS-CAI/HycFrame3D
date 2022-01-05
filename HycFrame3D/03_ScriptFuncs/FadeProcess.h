#pragma once

#include "UiAll.h"

void RegisterFadeProcess(ObjectFactory* _factory);

bool DeadFadeInit(UInteractComponent*);
void DeadFadeUpdate(UInteractComponent*, Timer&);
void DeadFadeDestory(UInteractComponent*);

bool SceneFadeInit(UInteractComponent*);
void SceneFadeUpdate(UInteractComponent*, Timer&);
void SceneFadeDestory(UInteractComponent*);

bool GetDeadFadeRunningFlg();
void SetDeadFadeRunningFlg(bool _flag);

bool GetSceneInFlg();
bool GetSceneOutFlg();
bool GetSceneOutFinish();
void SetSceneOutFlg(bool _flag);
