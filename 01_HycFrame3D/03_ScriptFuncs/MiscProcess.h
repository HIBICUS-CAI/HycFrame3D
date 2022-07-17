#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void RegisterMiscProcess(ObjectFactory* Factory);

bool DragonInit(AInteractComponent*);
void DragonUpdate(AInteractComponent*, const Timer&);
void DragonDestory(AInteractComponent*);

bool HillInfoInit(UInteractComponent*);
void HillInfoUpdate(UInteractComponent*, const Timer&);
void HillInfoDestory(UInteractComponent*);

bool ResultInit(UInteractComponent*);
void ResultUpdate(UInteractComponent*, const Timer&);
void ResultDestory(UInteractComponent*);

bool logoFadeInit(UInteractComponent*);
void logoFadeUpdate(UInteractComponent*, const Timer&);
void logoFadeDestory(UInteractComponent*);
