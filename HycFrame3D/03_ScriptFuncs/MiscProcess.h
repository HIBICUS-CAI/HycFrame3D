#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void RegisterMiscProcess(ObjectFactory* _factory);

bool DragonInit(AInteractComponent*);
void DragonUpdate(AInteractComponent*, Timer&);
void DragonDestory(AInteractComponent*);

bool HillInfoInit(UInteractComponent*);
void HillInfoUpdate(UInteractComponent*, Timer&);
void HillInfoDestory(UInteractComponent*);
