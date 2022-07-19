#pragma once

#include "UiAll.h"

void registerFadeProcess(ObjectFactory *Factory);

bool deadFadeInit(UInteractComponent *);
void deadFadeUpdate(UInteractComponent *, const Timer &);
void deadFadeDestory(UInteractComponent *);

bool sceneFadeInit(UInteractComponent *);
void sceneFadeUpdate(UInteractComponent *, const Timer &);
void sceneFadeDestory(UInteractComponent *);

bool getDeadFadeRunningFlg();
void setDeadFadeRunningFlg(bool Flag);

bool getSceneInFlg();
bool getSceneOutFlg();
bool getSceneOutFinish(UINT Filter = static_cast<UINT>(-1));
void setSceneOutFlg(bool Flag, UINT Filter = static_cast<UINT>(-1));
