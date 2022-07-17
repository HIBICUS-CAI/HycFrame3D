#pragma once

#include "ActorAll.h"

void RegisterSleepCrystal(ObjectFactory* _factory);

bool SCrystalInit(AInteractComponent*);
void SCrystalUpdate(AInteractComponent*, const Timer&);
void SCrystalDestory(AInteractComponent*);
