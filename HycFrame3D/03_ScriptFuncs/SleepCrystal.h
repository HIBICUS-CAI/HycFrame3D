#pragma once

#include "ActorAll.h"

void RegisterSleepCrystal(ObjectFactory* _factory);

bool SCrystalInit(AInteractComponent*);
void SCrystalUpdate(AInteractComponent*, Timer&);
void SCrystalDestory(AInteractComponent*);
