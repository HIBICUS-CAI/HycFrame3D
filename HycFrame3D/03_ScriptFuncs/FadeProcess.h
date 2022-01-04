#pragma once

#include "UiAll.h"

void RegisterFadeProcess(ObjectFactory* _factory);

bool DeadFadeInit(UInteractComponent*);
void DeadFadeUpdate(UInteractComponent*, Timer&);
void DeadFadeDestory(UInteractComponent*);

bool GetDeadFadeRunningFlg();
void SetDeadFadeRunningFlg(bool _flag);
