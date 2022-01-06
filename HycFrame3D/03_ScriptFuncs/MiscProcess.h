#pragma once

#include "ActorAll.h"

void RegisterMiscProcess(ObjectFactory* _factory);

bool DragonInit(AInteractComponent*);
void DragonUpdate(AInteractComponent*, Timer&);
void DragonDestory(AInteractComponent*);
