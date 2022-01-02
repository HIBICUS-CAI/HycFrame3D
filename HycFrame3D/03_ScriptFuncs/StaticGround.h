#pragma once

#include "ActorAll.h"

void RegisterStaticGround(ObjectFactory* _factory);

bool GoundInit(AInteractComponent*);
void GoundUpdate(AInteractComponent*, Timer&);
void GoundDestory(AInteractComponent*);
