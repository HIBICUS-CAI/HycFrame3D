#pragma once

#include "ActorAll.h"

void registerSleepCrystal(ObjectFactory *Factory);

bool sleepCrystalInit(AInteractComponent *);
void sleepCrystalUpdate(AInteractComponent *, const Timer &);
void sleepCrystalDestory(AInteractComponent *);
