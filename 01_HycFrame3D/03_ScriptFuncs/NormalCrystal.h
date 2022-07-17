#pragma once

#include "ActorAll.h"

void RegisterNormalCrystal(ObjectFactory* _factory);

bool NCrystalInit(AInteractComponent*);
void NCrystalUpdate(AInteractComponent*, const Timer&);
void NCrystalDestory(AInteractComponent*);
