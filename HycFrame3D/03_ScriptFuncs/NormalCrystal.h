#pragma once

#include "ActorAll.h"

void RegisterNormalCrystal(ObjectFactory* _factory);

bool NCrystalInit(AInteractComponent*);
void NCrystalUpdate(AInteractComponent*, Timer&);
void NCrystalDestory(AInteractComponent*);
