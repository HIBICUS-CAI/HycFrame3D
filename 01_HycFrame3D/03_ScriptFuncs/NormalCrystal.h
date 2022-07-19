#pragma once

#include "ActorAll.h"

void registerNormalCrystal(ObjectFactory *Factory);

bool normalCrystalInit(AInteractComponent *);
void normalCrystalUpdate(AInteractComponent *, const Timer &);
void normalCrystalDestory(AInteractComponent *);
