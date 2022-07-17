#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void RegisterMiscProcess(ObjectFactory* _factory);

bool DragonInit(AInteractComponent*);
void DragonUpdate(AInteractComponent*, const Timer&);
void DragonDestory(AInteractComponent*);

bool HillInfoInit(UInteractComponent*);
void HillInfoUpdate(UInteractComponent*, const Timer&);
void HillInfoDestory(UInteractComponent*);

bool ResultInit(UInteractComponent*);
void ResultUpdate(UInteractComponent*, const Timer&);
void ResultDestory(UInteractComponent*);

bool LogoFadeInit(UInteractComponent*);
void LogoFadeUpdate(UInteractComponent*, const Timer&);
void LogoFadeDestory(UInteractComponent*);
